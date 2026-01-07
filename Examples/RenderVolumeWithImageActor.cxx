// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

///#include "main.h"
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
#include "vtkMath.h"

template <class T>
void AppendSliceToVolume(T* slicePtr, T* volumePtr, size_t imageSize, int currentSlice)
{
  T* ptr = volumePtr + currentSlice * imageSize;
  memcpy(ptr, slicePtr, imageSize);
}

int main(int argc, char** argv)
{
  // create a rendering window and both renderers
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(renderer);
  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  vtkSmartPointer<vtkImageActor> ia = vtkSmartPointer<vtkImageActor>::New();
  renderer->AddActor(ia);

  // Create the volume: read the first image to get the x-y dimensions. Then append them all
  // together to create a volume.
  vtkSmartPointer<vtkJPEGReader> jpgReader = vtkSmartPointer<vtkJPEGReader>::New();
  jpgReader->SetFileName("C:/F/CV/Data/Test/Beach/beach1.jpg");
  jpgReader->Update();

  vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
  slice = jpgReader->GetOutput();
  int dims[3];
  slice->GetDimensions(dims);
  int scalarType = slice->GetScalarType();

  // Okay we have enough information to create the volume
  int numImages = 15;
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
  smoother->SetInputConnection(jpgReader->GetOutputPort());
  smoother->SetDimensionality(2);

  int currentSlice;
  size_t imageSize;
  for (currentSlice = 0; currentSlice < numImages; ++currentSlice)
    {
    switch (scalarType)
      {
      case VTK_UNSIGNED_CHAR:
        {
        smoother->SetStandardDeviation(currentSlice);
//        smoother->SetStandardDeviation(vtkMath::Random(1,10));
        smoother->Update();
        slice = jpgReader->GetOutput();
//        slice = smoother->GetOutput();
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

  // Loop through volume and display slices
  ia->SetInput(volume);
  ia->SetDisplayExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, numImages - 1);

  renderer->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(400, 400);

  int sliceNum;
  while (1)
    {
    cout << "Enter Slice Number" << "\n";
    cin >> currentSlice;
    ia->SetZSlice(currentSlice);
    renWin->Render();
//     for (currentSlice=0; currentSlice <numImages; ++currentSlice)
//       {
//       ia->SetZSlice(currentSlice);
//       renWin->Render();
//       }
    }

  iren->Start();

  return 0;
}
