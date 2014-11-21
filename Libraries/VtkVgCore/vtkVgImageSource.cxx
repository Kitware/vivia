/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgImageSource.h"

// VTK includes.
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageData.h"
#include "vtkImageFlip.h"
#include "vtkImageImport.h"
#include "vtkImageResample.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <vtksys/SystemTools.hxx>

// VIL includes.
#include <vil/vil_load.h>
#include <vil/vil_image_view.h>
#include <vil/vil_pixel_format.h>

// Standard VTK macro.
vtkStandardNewMacro(vtkVgImageSource);

//-----------------------------------------------------------------------------
class vtkVgImageSourceInternal
{
public:
  vtkVgImageSourceInternal(vtkVgImageSource* imgSrc);
  ~vtkVgImageSourceInternal();

  void          Init(const char* fileName);
  vtkImageData* VXLToVTKImageData();
  vtkImageData* DoOutputResolution(vtkImageData* inImg);

  std::string PrevFileName;
  vtkVgImageSource* VGImgSrc;
  vil_image_resource_sptr VILImageResrc;
};

//-----------------------------------------------------------------------------
vtkVgImageSourceInternal::vtkVgImageSourceInternal(vtkVgImageSource* imgSrc) :
  PrevFileName(""),
  VGImgSrc(imgSrc),
  VILImageResrc(0)
{
}

//-----------------------------------------------------------------------------
vtkVgImageSourceInternal::~vtkVgImageSourceInternal()
{
  this->VGImgSrc = 0;
}

//-----------------------------------------------------------------------------
void vtkVgImageSourceInternal::Init(const char* fileName)
{
  // Create a new resource if
  // 1. This is the first time.
  // 2. This is a different file than the last one.
  if (!this->VILImageResrc || (this->PrevFileName != std::string(fileName)))
    {
    this->VILImageResrc = vil_load_image_resource(fileName);
    this->PrevFileName = fileName;
    }
}

//-----------------------------------------------------------------------------
vtkImageData* vtkVgImageSourceInternal::VXLToVTKImageData()
{
  if (this->VILImageResrc == 0)
    {
    std::cerr << "Invalid image resource. " << std::endl;
    return 0;
    }

  const int* fullExtents = this->VGImgSrc->GetFullImageExtents();
  int fw = fullExtents[2] + 1;
  int fh = fullExtents[3] + 1;

  // @TODO: Take this out.
  std::cout  << "fw " << fw << std::endl;
  std::cout  << "fh " << fh << std::endl;

  int extents[4];
  int x, y, dw, dh;

  this->VGImgSrc->GetReadExtents(extents[0], extents[1], extents[2], extents[3]);

  // @TODO: Need to take this out.
  std::cout  << "extents: "
                << extents[0] << " "
                << extents[1] << " "
                << extents[2] << " "
                << extents[3] << std::endl;

  // For the very first time.
  if (extents[0] == -1 &&
      extents[1] == -1 &&
      extents[2] == -1 &&
      extents[3] == -1)
    {
    this->VGImgSrc->SetReadExtents(0, 0, (fw - 1), (fh - 1));
    this->VGImgSrc->GetReadExtents(extents[0], extents[1], extents[2], extents[3]);
    }

  // Check boundary conditions.
  for (int i = 0; i < 4; ++i)
    {
    if (extents[i] < 0)
      {
      std::cerr << "Cannot have negative image extents." << std::endl;
      return 0;
      }
    }

  if ((extents[0] > (fw - 1) || extents[2] > (fw - 1)) ||
      (extents[1] > (fh - 1) || extents[3] > (fh - 1)))
    {
    std::cerr << "Cannot have extents larger than the image extens."
                 << std::endl;

    return 0;
    }

  dw  = extents[2] - extents[0] + 1;
  dh  = extents[3] - extents[1] + 1;

  x = extents[0];

  // In VXL world +Y is down.
  y = fh - (extents[3] + 1);

  // @TODO: Need to take this out.
  std::cout  << "x " << x << " dw " << dw << " y " << y << " dh " << dh << std::endl;

  vil_image_view<vxl_byte> image;
  image = VILImageResrc->get_view(x, dw, y, dh);

  if (!image)
    {
    std::cerr << "Invalid image view." << std::endl;
    return 0;
    }

  int w   = static_cast<int>(image.ni());
  int h   = static_cast<int>(image.nj());
  int np  = static_cast<int>(image.nplanes());

  // @NOTE: This is how you query pixel format on the incoming data.
//  VILImageResrc->pixel_format();

  // Currently we handle images with 1 - 3 components. (R, RG, RGB).
  if (np < 1 || np > 3)
    {
    std::cerr << "Cannot handle image with 0 or more than 3 planes. "
                 << std::endl;
    return 0;
    }

  if (w < 0 || h < 0)
    {
    std::cerr << "Invalid image. " << std::endl;
    return 0;
    }

  if (w == 0 && h == 0)
    {
    std::cerr << "Invalid image. Width and height values are 0."
                 << std::endl;
    return 0;
    }

  int pf = VILImageResrc->pixel_format();
  std::cout << " pixel format " << VILImageResrc->pixel_format() << std::endl;

  // @TODO: Need to take this out.
  std::cout  << "w=" << w << std::endl;
  std::cout  << "h=" << h << std::endl;
  std::cout  << "nj=" << VILImageResrc->nj() << std::endl;
  std::cout  << "nplanes=" << image.nplanes() << std::endl;
  std::cout  << "is_contiguous=" << image.is_contiguous() << std::endl;
  std::cout  << "size_bytes=" << image.size_bytes() << std::endl;
  std::cout  << "istep=" << image.istep() << std::endl;
  std::cout  << "jstep=" << image.jstep() << std::endl;
  std::cout  << "planestep=" << image.planestep() << std::endl;

  // @Note: This is somewhat limiting but sufficient as of now.
  int sdt = -1;
  if (pf == VIL_PIXEL_FORMAT_BYTE)
    {
    sdt = VTK_UNSIGNED_CHAR;
    }
  else if (pf == VIL_PIXEL_FORMAT_SBYTE)
    {
    sdt = VTK_UNSIGNED_SHORT;
    }
  else
    {
    std::cout << "Scalar data type " << sdt << " is not supported. "
                 << std::endl;
    return 0;
    }

  vtkSmartPointer<vtkImageImport> importerRed(vtkSmartPointer<vtkImageImport>::New());
  importerRed->SetDataScalarType(sdt);
  importerRed->SetNumberOfScalarComponents(1);
  importerRed->SetImportVoidPointer(image.top_left_ptr());
  importerRed->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
  importerRed->SetDataExtentToWholeExtent();
  importerRed->Modified();
  importerRed->Update();

  vtkSmartPointer<vtkImageImport> importerGreen(0);
  vtkSmartPointer<vtkImageImport> importerBlue(0);

  if (np > 1)
    {
    importerGreen = vtkSmartPointer<vtkImageImport>::New();
    importerGreen->SetDataScalarType(sdt);
    importerGreen->SetNumberOfScalarComponents(1);
    importerGreen->SetImportVoidPointer(image.top_left_ptr() + image.planestep());
    importerGreen->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
    importerGreen->SetDataExtentToWholeExtent();
    importerGreen->Modified();
    importerGreen->Update();
    }

  if (np > 2)
    {
    importerBlue = vtkSmartPointer<vtkImageImport>::New();
    importerBlue->SetDataScalarType(sdt);
    importerBlue->SetNumberOfScalarComponents(1);
    importerBlue->SetImportVoidPointer(image.top_left_ptr() + 2 * image.planestep());
    importerBlue->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
    importerBlue->SetDataExtentToWholeExtent();
    importerBlue->Modified();
    importerBlue->Update();
    }

  vtkSmartPointer<vtkImageAppendComponents> compnts(
    vtkSmartPointer<vtkImageAppendComponents>::New());
  compnts->SetInputConnection(importerRed->GetOutputPort());

  if (importerGreen)
    {
    compnts->AddInputConnection(importerGreen->GetOutputPort());
    }
  if (importerBlue)
    {
    compnts->AddInputConnection(importerBlue->GetOutputPort());
    }

  compnts->Update();

  vtkSmartPointer<vtkImageFlip> flip(vtkSmartPointer<vtkImageFlip>::New());
  flip->SetInputConnection(compnts->GetOutputPort());
  flip->SetFlipAboutOrigin(false);
  flip->SetFilteredAxes(1);
  flip->Update();

  vtkImageData* img(vtkImageData::New());
  img->ShallowCopy(flip->GetOutput());
  return img;
}

//----------------------------------------------------------------------------
vtkImageData* vtkVgImageSourceInternal::DoOutputResolution(vtkImageData* inImg)
{
  if (!inImg)
    {
    std::cerr << "Invalid input image data." << std::endl;
    return 0;
    }

  if (!this->VGImgSrc->GetUseOutputResolution())
    {
    std::cerr << "UseOutputResolution flag is set to false." << std::endl;
    return 0;
    }

  int mx, my;
  this->VGImgSrc->GetOutputResolution(mx, my);

  if (mx < 0 || my < 0)
    {
    std::cerr << "Cannot have negavitve values for the resolution."
                 << std::endl;
    return 0;
    }

  const int* dims = inImg->GetDimensions();
  if (!dims)
    {
    std::cerr << "Unable to query extents of the image." << std::endl;
    return 0;
    }

  int w = dims[0];
  int h = dims[1];

  // Easy to use short name.
  typedef vtkSmartPointer<vtkImageResample> vtkImageResampleRef;

  vtkImageResampleRef imgRsmpl(vtkImageResampleRef::New());
  imgRsmpl->SetInputData(inImg);
  imgRsmpl->SetAxisMagnificationFactor(0, static_cast<double>(mx) / w);
  imgRsmpl->SetAxisMagnificationFactor(1, static_cast<double>(my) / h);
  imgRsmpl->Update();

  vtkImageData* output(vtkImageData::New());
  output->ShallowCopy(imgRsmpl->GetOutput());

  return output;
}

//----------------------------------------------------------------------------
vtkVgImageSource::vtkVgImageSource() : vtkVgBaseImageSource(),
  Implementation(0)
{
  // Initialize to some resonable default values.
  for (int i = 0; i < 4; ++i)
    {
    FullImageExtents[i] = -1;
    }

  this->Implementation = new vtkVgImageSourceInternal(this);

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkVgImageSource::~vtkVgImageSource()
{
  delete this->Implementation;

  this->Implementation = 0;
}

//----------------------------------------------------------------------------
void vtkVgImageSource::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  // @TODO: Implement this.
}

//----------------------------------------------------------------------------
void vtkVgImageSource::GetDimensions(int dim[2])
{
  if (this->GetFullImageExtents())
    {
    dim[0] = this->FullImageExtents[2] + 1;
    dim[1] = this->FullImageExtents[3] + 1;
    }
  else
    {
    dim[0] = 0;
    dim[1] = 0;
    }
}

//----------------------------------------------------------------------------
const int* vtkVgImageSource::GetFullImageExtents()
{
  if (this->FileName)
    {
    this->Implementation->Init(this->FileName);

    if (this->Implementation->VGImgSrc)
      {
      this->FullImageExtents[0] = 0;
      this->FullImageExtents[1] = 0;
      this->FullImageExtents[2] =
        static_cast<int>(this->Implementation->VILImageResrc->ni()) - 1;
      this->FullImageExtents[3] =
        static_cast<int>(this->Implementation->VILImageResrc->nj()) - 1;

      return this->FullImageExtents;
      }
    else
      {
      vtkErrorMacro("Unable to query the image full extents");
      return 0;
      }
    }
  else
    {
    vtkErrorMacro("Unable to calculate full image extents "
                  << "without a valid input file name.");
    return 0;
    }
}

//----------------------------------------------------------------------------
bool vtkVgImageSource::CanRead(const std::string& source)
{
  std::string ciSource = vtksys::SystemTools::LowerCase(source);

  // Check if it is a filename or if the argument is just an extension.
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(ciSource);
  if (ext.compare(".jp2") == 0 || (ciSource.compare("jp2") == 0) ||
      (ciSource.compare(".jp2") == 0))
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkVgImageSource::GetShortDescription()
{
  return "Reader for PNG file format";
}

//----------------------------------------------------------------------------
std::string vtkVgImageSource::GetLongDescription()
{
  return this->GetShortDescription();
}

//----------------------------------------------------------------------------
vtkVgBaseImageSource* vtkVgImageSource::Create()
{
  return vtkVgImageSource::New();
}

//----------------------------------------------------------------------------
int vtkVgImageSource::RequestInformation(vtkInformation* vtkNotUsed(request),
                                         vtkInformationVector** vtkNotUsed(inputVector),
                                         vtkInformationVector* outputVector)
{
  // Get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  int dataExtent[6];
  double spacing[3] = {1, 1, 1};
  double origin[3] = {0, 0, 0};

  if (this->GetUseOutputResolution())
    {
    dataExtent[0] = 0;
    dataExtent[1] = this->OutputResolution[0] - 1;
    dataExtent[2] = 0;
    dataExtent[3] = this->OutputResolution[1] - 1;
    dataExtent[4] = 0;
    dataExtent[5] = 0;
    }
  else
    {
    bool getFullExtents(false);
    for (int i = 0; i < 4; ++i)
      {
      if (this->ReadExtents[i] == -1)
        {
        getFullExtents = true;
        break;
        }
      }

    if (getFullExtents)
      {
      const int* extents = this->GetFullImageExtents();
      this->SetReadExtents(extents[0], extents[1],
                           extents[2], extents[3]);
      }

    origin[0] = this->ReadExtents[0];
    origin[1] = this->ReadExtents[1];
    dataExtent[0] = 0;
    dataExtent[1] = this->ReadExtents[2] - this->ReadExtents[0] - 1;
    dataExtent[2] = 0;
    dataExtent[3] = this->ReadExtents[3] - this->ReadExtents[1] - 1;
    dataExtent[4] = 0;
    dataExtent[5] = 0;
    }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               dataExtent, 6);
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(),  origin,  3);

  return 1;
}

//----------------------------------------------------------------------------
int vtkVgImageSource::RequestData(vtkInformation* vtkNotUsed(request),
                                  vtkInformationVector** vtkNotUsed(inputVector),
                                  vtkInformationVector* outputVector)
{
  if (!outputVector)
    {
    vtkErrorMacro("Invalid output information vector.") ;
    return 1;
    }

  if (!this->FileName)
    {
    vtkErrorMacro("Requires valid input file name.") ;
    return 1;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information object.");
    return 1;
    }

  vtkDataObject* dataObj = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if (!dataObj)
    {
    vtkErrorMacro("Invalid output data object.");
    return 1;
    }

  vtkImageData* outputImage = vtkImageData::SafeDownCast(dataObj);
  if (!outputImage)
    {
    vtkErrorMacro("Output data object is not an image data object.");
    return 1;
    }

  this->Implementation->Init(this->FileName);
  vtkSmartPointer<vtkImageData> output;

  if (this->GetUseOutputResolution())
    {
    vtkSmartPointer<vtkImageData> input;
    input.TakeReference(this->Implementation->VXLToVTKImageData());
    output.TakeReference(this->Implementation->DoOutputResolution(input));
    }
  else
    {
    output.TakeReference(this->Implementation->VXLToVTKImageData());
    }

  outputImage->ShallowCopy(output);
  outputImage->SetOrigin(this->ReadExtents[0], this->ReadExtents[1], 0);

  return 1;
}
