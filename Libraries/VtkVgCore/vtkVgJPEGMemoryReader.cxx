// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgJPEGMemoryReader.h"

#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

extern "C" {
#include "vtk_jpeg.h"
#include "jmem_src.h"
#include <setjmp.h>
}

vtkStandardNewMacro(vtkVgJPEGMemoryReader);

// create an error handler for jpeg that
// can longjmp out of the jpeg library
struct vtk_jpeg_error_mgr
{
  struct jpeg_error_mgr pub;    /* "public" fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
  vtkVgJPEGMemoryReader* JPEGReader;
};

// this is called on jpeg error conditions
extern "C" void vtk_jpeg_error_exit_mr(j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  vtk_jpeg_error_mgr* err = reinterpret_cast<vtk_jpeg_error_mgr*>(cinfo->err);

  /* Return control to the setjmp point */
  longjmp(err->setjmp_buffer, 1);
}

extern "C" void vtk_jpeg_output_message_mr(j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  (*cinfo->err->format_message)(cinfo, buffer);
  vtk_jpeg_error_mgr* err = reinterpret_cast<vtk_jpeg_error_mgr*>(cinfo->err);
  vtkWarningWithObjectMacro(err->JPEGReader,
                            "libjpeg error: " << buffer);
}

#ifdef _MSC_VER
// Let us get rid of this funny warning on /W4:
// warning C4611: interaction between '_setjmp' and C++ object
// destruction is non-portable
#pragma warning(disable : 4611)
#endif

void vtkVgJPEGMemoryReader::ExecuteInformation()
{
  this->ComputeInternalFileName(this->DataExtent[4]);
  if (this->InternalFileName == NULL)
    {
    return;
    }

  if (!this->BufferSize || !this->Buffer)
    {
    vtkErrorWithObjectMacro(this,
                            "No memory buffer set "
                            << this->InternalFileName);
    return;
    }

  // create jpeg decompression object and error handler
  struct jpeg_decompress_struct cinfo;
  struct vtk_jpeg_error_mgr jerr;
  jerr.JPEGReader = this;

  cinfo.err = jpeg_std_error(&jerr.pub);
  // for any jpeg error call vtk_jpeg_error_exit
  jerr.pub.error_exit = vtk_jpeg_error_exit_mr;
  // for any output message call vtk_jpeg_output_message
  jerr.pub.output_message = vtk_jpeg_output_message_mr;
  if (setjmp(jerr.setjmp_buffer))
    {
    // clean up
    jpeg_destroy_decompress(&cinfo);
    // close the file
    // this is not a valid jpeg file
    vtkErrorWithObjectMacro(this, "libjpeg could not read file: "
                            << this->InternalFileName);
    return;
    }
  jpeg_create_decompress(&cinfo);

  // set the source file
  jpeg_memory_src(&cinfo, this->Buffer, this->BufferSize);

  // read the header
  jpeg_read_header(&cinfo, TRUE);

  // force the output image size to be calculated (we could have used
  // cinfo.image_height etc. but that would preclude using libjpeg's
  // ability to scale an image on input).
  jpeg_calc_output_dimensions(&cinfo);

  // pull out the width/height, etc.
  this->DataExtent[0] = 0;
  this->DataExtent[1] = cinfo.output_width - 1;
  this->DataExtent[2] = 0;
  this->DataExtent[3] = cinfo.output_height - 1;

  this->SetDataScalarTypeToUnsignedChar();
  this->SetNumberOfScalarComponents(cinfo.output_components);

  this->vtkImageReader2::ExecuteInformation();

  // close the file
  jpeg_destroy_decompress(&cinfo);
}

template <class OT>
void vtkJPEGMemoryReaderUpdate2(vtkVgJPEGMemoryReader* self, OT* outPtr,
                                int* outExt, vtkIdType* outInc, long)
{
  unsigned int ui;
  int i;

  // create jpeg decompression object and error handler
  struct jpeg_decompress_struct cinfo;
  struct vtk_jpeg_error_mgr jerr;
  jerr.JPEGReader = self;

  cinfo.err = jpeg_std_error(&jerr.pub);
  // for any jpeg error call vtk_jpeg_error_exit
  jerr.pub.error_exit = vtk_jpeg_error_exit_mr;
  // for any output message call vtk_jpeg_output_message
  jerr.pub.output_message = vtk_jpeg_output_message_mr;
  if (setjmp(jerr.setjmp_buffer))
    {
    // clean up
    jpeg_destroy_decompress(&cinfo);
    // close the file
    vtkErrorWithObjectMacro(self, "libjpeg could not read file: "
                            << self->GetInternalFileName());
    // this is not a valid jpeg file
    return;
    }
  jpeg_create_decompress(&cinfo);

  // set the source file
  jpeg_memory_src(&cinfo, self->Buffer, self->BufferSize);

  // read the header
  jpeg_read_header(&cinfo, TRUE);

  // prepare to read the bulk data
  jpeg_start_decompress(&cinfo);

  int rowbytes = cinfo.output_components * cinfo.output_width;
  unsigned char* tempImage = new unsigned char [rowbytes * cinfo.output_height];
  JSAMPROW* row_pointers = new JSAMPROW [cinfo.output_height];
  for (ui = 0; ui < cinfo.output_height; ++ui)
    {
    row_pointers[ui] = tempImage + rowbytes * ui;
    }

  // read the bulk data
  unsigned int remainingRows = cinfo.output_height;
  while (cinfo.output_scanline < cinfo.output_height)
    {
    remainingRows = cinfo.output_height - cinfo.output_scanline;
    jpeg_read_scanlines(&cinfo, &row_pointers[cinfo.output_scanline],
                        remainingRows);
    }

  // finish the decompression step
  jpeg_finish_decompress(&cinfo);

  // destroy the decompression object
  jpeg_destroy_decompress(&cinfo);

  // copy the data into the outPtr
  OT* outPtr2;
  outPtr2 = outPtr;
  long outSize = cinfo.output_components * (outExt[1] - outExt[0] + 1);
  for (i = outExt[2]; i <= outExt[3]; ++i)
    {
    memcpy(outPtr2,
           row_pointers[cinfo.output_height - i - 1]
           + outExt[0]*cinfo.output_components,
           outSize);
    outPtr2 += outInc[1];
    }
  delete [] tempImage;
  delete [] row_pointers;

}

//----------------------------------------------------------------------------
// This function reads in one data of data.
// templated to handle different data types.
template <class OT>
void vtkJPEGMemoryReaderUpdate(vtkVgJPEGMemoryReader* self, vtkImageData* data, OT* outPtr)
{
  vtkIdType outIncr[3];
  int outExtent[6];
  OT* outPtr2;

  data->GetExtent(outExtent);
  data->GetIncrements(outIncr);

  long pixSize = data->GetNumberOfScalarComponents() * sizeof(OT);

  outPtr2 = outPtr;
  int idx2;
  for (idx2 = outExtent[4]; idx2 <= outExtent[5]; ++idx2)
    {
    self->ComputeInternalFileName(idx2);
    // read in a JPEG file
    vtkJPEGMemoryReaderUpdate2(self, outPtr2, outExtent, outIncr, pixSize);
    self->UpdateProgress((idx2 - outExtent[4]) /
                         (outExtent[5] - outExtent[4] + 1.0));
    outPtr2 += outIncr[2];
    }
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkVgJPEGMemoryReader::ExecuteDataWithInformation(vtkDataObject* output,
                                                       vtkInformation *outInfo)
{
  vtkImageData* data = this->AllocateOutputData(output, outInfo);

  if (this->InternalFileName == NULL)
    {
    vtkErrorMacro(<< "Either a FileName or FilePrefix must be specified.");
    return;
    }

  this->ComputeDataIncrements();

  data->GetPointData()->GetScalars()->SetName("JPEGImage");

  // Call the correct templated function for the output
  void* outPtr;

  // Call the correct templated function for the input
  outPtr = data->GetScalarPointer();
  switch (data->GetScalarType())
    {
    vtkTemplateMacro(vtkJPEGMemoryReaderUpdate(this, data, (VTK_TT*)(outPtr)));
    default:
      vtkErrorMacro(<< "UpdateFromFile: Unknown data type");
    }
}

#ifdef _MSC_VER
// Put the warning back
#pragma warning(default : 4611)
#endif

//----------------------------------------------------------------------------
void vtkVgJPEGMemoryReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
