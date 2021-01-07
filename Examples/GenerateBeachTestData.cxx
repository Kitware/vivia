// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkImageData.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkJPEGReader.h"
#include "vtkImageActor.h"
#include "vtkCamera.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include "vtkUnsignedCharArray.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageMagnify.h"
#include "vtkImageWrapPad.h"
#include "vtkMath.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkJPEGWriter.h"
#include "vtkExtractVOI.h"

template <class T>
void AppendSliceToVolume(T* slicePtr, T* volumePtr, size_t imageSize, int currentSlice)
{
  T* ptr = volumePtr + currentSlice * imageSize;
  memcpy(ptr, slicePtr, imageSize);
}

int main(int argc, char** argv)
{
  // Global controls
  int magFactor = 5;
  int numImages = 100;

  // Create the volume: read the first image to get the x-y dimensions. Then append them all
  // together to create a volume.
  vtkSmartPointer<vtkJPEGReader> jpgReader = vtkSmartPointer<vtkJPEGReader>::New();
  jpgReader->SetFileName("C:/F/CV/Data/Test/Beach/beach1.jpg");
  vtkSmartPointer<vtkImageWrapPad> pad = vtkSmartPointer<vtkImageWrapPad>::New();
  pad->SetInputConnection(jpgReader->GetOutputPort());
  pad->SetOutputWholeExtent(0, 999, 0, 999, 0, 0);
  pad->Update();

  vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
  slice = pad->GetOutput();
  int dims[3];
  slice->GetDimensions(dims);
  int scalarType = slice->GetScalarType();

  // Okay we have enough information to create the volume
  vtkSmartPointer<vtkImageData> volume = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkDataArray> scalars = slice->GetPointData()->GetScalars()->NewInstance();
  scalars->UnRegister(NULL);
  int numComps = slice->GetPointData()->GetScalars()->GetNumberOfComponents();
  scalars->SetNumberOfComponents(numComps);
  scalars->SetNumberOfTuples(dims[0]*dims[1]*numImages);
  volume->GetPointData()->SetScalars(scalars);
  volume->SetExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, numImages - 1);
  volume->SetWholeExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, numImages - 1);

  // Now append the slices into the volume. Use pointers / memcpy for speed
  vtkSmartPointer<vtkImageGaussianSmooth> smoother =
    vtkSmartPointer<vtkImageGaussianSmooth>::New();
  smoother->SetInputConnection(pad->GetOutputPort());
  smoother->SetDimensionality(2);

  int currentSlice;
  size_t imageSize;
  for (currentSlice = 0; currentSlice < numImages; ++currentSlice)
    {
    switch (scalarType)
      {
      case VTK_UNSIGNED_CHAR:
        {
        smoother->SetStandardDeviation(10 * currentSlice / numImages);
        smoother->Update();
        slice = smoother->GetOutput();

        imageSize = dims[0] * dims[1] * sizeof(unsigned char) * numComps;
        unsigned char* volumePtr = vtkUnsignedCharArray::SafeDownCast(scalars)->GetPointer(0);
        unsigned char* slicePtr = vtkUnsignedCharArray::SafeDownCast(
          slice->GetPointData()->GetScalars())->GetPointer(0);
        AppendSliceToVolume(slicePtr, volumePtr, imageSize, currentSlice);
        }
      break;

      default:
        cout << "Bad data type\n";
      }
    }

  // Write out high-res data file
  vtkSmartPointer<vtkExtractVOI> subsample =
    vtkSmartPointer<vtkExtractVOI>::New();
  subsample->SetInput(volume);
  vtkSmartPointer<vtkJPEGWriter> writer =
    vtkSmartPointer<vtkJPEGWriter>::New();
  writer->SetInputConnection(subsample->GetOutputPort());
  char outputStr[256];
  for (int k = 0; k < numImages; ++k)
    {
    subsample->SetVOI(0, dims[0] - 1, 0, dims[1] - 1, k, k);
    subsample->Update();
    vtkImageData* subOut = subsample->GetOutput();
    sprintf(outputStr, "c:/f/CV/Data/Test/Beach/HighResData_%i.jpg", k);
    writer->SetFileName(outputStr);
    writer->Write();
    }

  // Write out low-res data file
  vtkSmartPointer<vtkExtractVOI> subsample2 = vtkSmartPointer<vtkExtractVOI>::New();
  subsample2->SetInput(volume);
  subsample2->SetSampleRate(10, 10, 1);
  vtkSmartPointer<vtkXMLImageDataWriter> writer2 =
    vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer2->SetInputConnection(subsample2->GetOutputPort());
  writer2->SetFileName("c:/f/CV/Data/Test/Beach/LowResData.vti");
  writer2->Write();

  return 0;
}
