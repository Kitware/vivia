// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// VTKExtensions include.
#include "vtkVgAppendImageData.h"
#include "vtkVgImageSource.h"
#include "vtkVgMultiResJpgImageReader2.h"

//
#include <vtkConditionVariable.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMultiThreader.h>
#include <vtkObjectFactory.h>
#include <vtkMutexLock.h>
#include <vtkPNGReader.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtksys/SystemTools.hxx>

// STL includes.
#include <string>
#include <sstream>

vtkStandardNewMacro(vtkVgAppendImageData);

// Thread stuff.
//-----------------------------------------------------------------------------
VTK_THREAD_RETURN_TYPE vtkAppendImageDataThreadStart(void* arg)
{
  vtkVgAppendImageData* self;
  self = static_cast<vtkVgAppendImageData*>
         (static_cast<vtkMultiThreader::ThreadInfo*>(arg)->UserData);
  self->Start();
  return VTK_THREAD_RETURN_VALUE;
}

// This class hides the implementation details.
//-----------------------------------------------------------------------------
class vtkAppendImageDataInternal
{
public:
  vtkAppendImageDataInternal(vtkVgAppendImageData* aidPtr);
  ~vtkAppendImageDataInternal();

  vtkSmartPointer<vtkImageData> ImageVolume;
  vtkVgAppendImageData*           Interface;
};

//-----------------------------------------------------------------------------
vtkAppendImageDataInternal::vtkAppendImageDataInternal(
  vtkVgAppendImageData* aidPtr) :
  Interface(aidPtr)
{
}

//-----------------------------------------------------------------------------
vtkAppendImageDataInternal::~vtkAppendImageDataInternal()
{
}

//-----------------------------------------------------------------------------
vtkVgAppendImageData::vtkVgAppendImageData() :
  vtkImageAlgorithm()
{
  this->StopThread          = false;
  this->UseOutputResolution = false;

  this->SetBaseFileName(0);
  this->SetBaseFileNamePrefix(0);
  this->SetBaseFileNameNum(0);

  this->NoOfFiles       = -1;
  this->BaseNum         = -1;
  this->WorkerThreadId  = -1;
  this->StateArray      =  0;

  for (int i = 0; i < 4; ++i)
    {
    ReadExtents[i]      = -1;
    }

  OutputResolution[0] = -1;
  OutputResolution[1] = -1;

  this->Threader = vtkMultiThreader::New();
  this->Threader->SetNumberOfThreads(1);

  this->ProcessingLock = vtkMutexLock::New();

  this->ConditionLock = vtkMutexLock::New();
  this->Condition     = vtkConditionVariable::New();

  this->Lock          = vtkMutexLock::New();

  this->Implementation = new vtkAppendImageDataInternal(this);

  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkVgAppendImageData::~vtkVgAppendImageData()
{
  std::cout << "~vtkVgAppendImageData" << std::endl;

  this->SetBaseFileName(0);
  this->SetBaseFileNamePrefix(0);
  this->SetBaseFileNameNum(0);

  this->VGImageReader->Delete();

  this->Threader->Delete();
  this->ProcessingLock->Delete();
  this->ConditionLock->Delete();
  this->Condition->Delete();

  this->Lock->Delete();

  delete this->Implementation;
}

//-----------------------------------------------------------------------------
//vtkImageData* vtkVgAppendImageData::GetData()
//{
//  return this->Implementation->ImageVolume;
//}

//-----------------------------------------------------------------------------
void vtkVgAppendImageData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "StopThread: " << this->StopThread << std::endl;
  os << indent << "BaseFileName: " << (this->BaseFileName != NULL
    ? this->BaseFileName : "NULL") << std::endl;
  os << indent << "BaseFileNamePrefix: " << (this->BaseFileNamePrefix != NULL
    ? this->BaseFileName : "NULL") << std::endl;
  os << indent << "BaseFileNameNum: " << (this->BaseFileNameNum != NULL
    ? this->BaseFileName : "NULL") << std::endl;
  os << indent << "NoOfFiles: " << NoOfFiles << std::endl;
  os << indent << "BaseNum: " << BaseNum << std::endl;
  os << indent << "WorkerThreadId: " << WorkerThreadId << std::endl;
}

//-----------------------------------------------------------------------------
void vtkVgAppendImageData::GetOutputResolution(int &w, int &h) const
{
  w = this->OutputResolution[0];
  h = this->OutputResolution[1];
  return;
}

//-----------------------------------------------------------------------------
void vtkVgAppendImageData::SetOutputResolution(int w, int h)
{
  if (w < 0)
    {
    // Error.
    return;
    }

  if (h < 0)
    {
    // Error.
    return;
    }

  this->OutputResolution[0] = w;
  this->OutputResolution[1] = h;

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgAppendImageData::UpdateStateInfo(int* stArr)
{
  if (!stArr)
    {
    std::cerr << "Incoming array is NULL or Invalid." << std::endl;
    return;
    }

  this->ProcessingLock->Lock();

  for (int i = 0; i < this->NoOfFiles; ++i)
    {
    stArr[i] = StateArray[i];
    }

  this->ProcessingLock->Unlock();
}

//-----------------------------------------------------------------------------
int vtkVgAppendImageData::RequestInformation(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  // Making sure to initialize VGImageReader.
  this->Init();

  // Get the info object.
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  int dataExtent[6] = { -1, -1, -1, -1, -1, -1};
  double spacing[3] = {1, 1, 1};
  double origin[3] = {0, 0, 0};

  if (this->GetUseOutputResolution())
    {
    dataExtent[0] = 0;
    dataExtent[1] = this->OutputResolution[0] - 1;
    dataExtent[2] = 0;
    dataExtent[3] = this->OutputResolution[1] - 1;
    dataExtent[4] = 0;
    dataExtent[5] = this->NoOfFiles - 1;
    }
  else
    {
    // Assuming that read extens are not bigger than the actual dimensions of
    // the image.
    // @TODO: Fix this assumption later.
    //
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
      vtkImageData* image = this->VGImageReader->GetOutput();
      if (image)
        {
        int extents[6];
        image->GetExtent(extents);
        this->SetReadExtents(extents[0], extents[1],
                             extents[2], extents[3]);
        }
      }

    origin[0] = this->ReadExtents[0];
    origin[1] = this->ReadExtents[1];
    dataExtent[0] = 0;
    dataExtent[1] = this->ReadExtents[2] - this->ReadExtents[0] - 1;
    dataExtent[2] = 0;
    dataExtent[3] = this->ReadExtents[3] - this->ReadExtents[1] - 1;
    dataExtent[4] = 0;
    dataExtent[5] = this->NoOfFiles - 1;
    }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               dataExtent, 6);
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(),  origin,  3);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkVgAppendImageData::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  // First stop the last thread running.
  // @NOTE: May be I need to forcefully kill the thread here.
  this->Stop();
  this->Finish();

  // Pipeline stuff.
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (!outInfo)
    {
    vtkErrorMacro("Invalid output information.\n");
    return 1;
    }

  vtkDataObject* outDataObj = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if (!outDataObj)
    {
    vtkErrorMacro("Invalid output data object.\n");
    return 1;
    }

  vtkImageData* outImageData = vtkImageData::SafeDownCast(outDataObj);
  if (!outImageData)
    {
    vtkErrorMacro("Invalid output image data.\n")
    return 1;
    }

  // First extract the required ivars.
  this->ComputerInternalFileName();

  // Initialize the state array with false (0) value.
  this->ProcessingLock->Lock();
  this->StateArray = new int[this->NoOfFiles];
  for (int i = 0; i < this->NoOfFiles; ++i)
    {
    this->StateArray[i] = 0;
    }
  this->ProcessingLock->Unlock();

  this->Implementation->ImageVolume = vtkSmartPointer<vtkImageData>::New();

  // Read the very first file for the meta data.
  //@NOTE: This has been moved to Init() which will be called during RequestInformation.
  vtkSmartPointer<vtkImageData> tempImageData(vtkSmartPointer<vtkImageData>::New());
//  this->VGImageReader->SetFileName(this->BaseFileName);
//  this->VGImageReader->SetUseOutputResolution(this->UseOutputResolution);
//  this->VGImageReader->SetReadExtents(this->ReadExtents);
//  this->VGImageReader->SetOutputResolution(this->OutputResolution[0],
//                                           this->OutputResolution[1]);
//  this->VGImageReader->Update();

  tempImageData->ShallowCopy(this->VGImageReader->GetOutput());

  int dims[] = {0, 0};
  if (tempImageData)
    {
    tempImageData->GetDimensions(dims);
    }
  else
    {
    std::cout << "Failed to read the image. " << std::endl;
    return 1;
    }

  // @TODO Fix this
  int wext[6] = {0, dims[0] - 1,
                 0, dims[1] - 1,
                 0, this->NoOfFiles - 1};
  outInfo->Set(
    vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wext, 6);

  // @NOTE: Do we need to set the same origin as the original data.
//  this->Implementation->ImageVolume->SetScalarType(tempImageData->GetScalarType());
//  this->Implementation->ImageVolume->SetNumberOfScalarComponents(
//    tempImageData->GetNumberOfScalarComponents());
//  this->Implementation->ImageVolume->SetOrigin(tempImageData->GetOrigin());
//  this->Implementation->ImageVolume->SetSpacing(tempImageData->GetSpacing());
//  this->Implementation->ImageVolume->AllocateScalars();

  // Here we are spawing a new thread.
//  this->WorkerThreadId =
//    this->Threader->SpawnThread(vtkAppendImageDataThreadStart, this);

//  outImageData->ShallowCopy(this->Implementation->ImageVolume);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkVgAppendImageData::Start()
{
  int currIndex = this->BaseNum;

  int lenBaseStr = static_cast<int>(strlen(this->BaseFileNameNum));

  // Loop.
  while (currIndex < (this->BaseNum + this->NoOfFiles))
    {
    this->Lock->Lock();
    if (this->StopThread)
      {
      this->Condition->Broadcast();
      this->Lock->Unlock();
      return;
      }
    this->Lock->Unlock();

    int sliceNo = currIndex - this->BaseNum;

    // If already done reading skip.
    if (StateArray[sliceNo])
      {
      continue;
      }

    std::ostringstream oss;
    oss << currIndex;

    std::string indexStr = oss.str();
    int lenIndexStr = static_cast<int>(indexStr.length());

    for (int i = 0; i < (lenBaseStr - lenIndexStr); ++i)
      {
      indexStr = std::string("0" + indexStr);
      }

    // Get the complete path of the next file in line.
    std::string nextInputFileName = std::string(this->BaseFileNamePrefix
      + indexStr
      + vtksys::SystemTools::GetFilenameExtension(this->BaseFileName));

    std::cout << "Processing file " << nextInputFileName << std::endl;

    this->VGImageReader->SetFileName(nextInputFileName.c_str());
    this->VGImageReader->SetUseOutputResolution(this->UseOutputResolution);
    this->VGImageReader->SetReadExtents(this->ReadExtents);
    this->VGImageReader->SetOutputResolution(this->OutputResolution[0],
                                             this->OutputResolution[1]);
    this->VGImageReader->Update();
    vtkImageData* outImgData = this->VGImageReader->GetOutput();

    // Find the address.
    int dims[3];
    int dataSize = this->Implementation->ImageVolume->GetScalarSize();
    this->Implementation->ImageVolume->GetDimensions(dims);
    size_t copySize = dims[0] * dims[1] * dataSize *
                      outImgData->GetNumberOfScalarComponents();

    memcpy(this->Implementation->ImageVolume->GetScalarPointer(0, 0, sliceNo),
           outImgData->GetScalarPointer(),
           copySize);

    this->ProcessingLock->Lock();
    this->StateArray[sliceNo] = 1;
    this->ProcessingLock->Unlock();

    ++currIndex;
    }

  this->Condition->Broadcast();
}

//-----------------------------------------------------------------------------
void vtkVgAppendImageData::Stop()
{
  // Terminate the thread now.
  if (this->WorkerThreadId != -1
    && this->Threader->IsThreadActive(this->WorkerThreadId))
    {
    this->Lock->Lock();
    this->StopThread = true;
    this->Lock->Unlock();
    }
}

//-----------------------------------------------------------------------------
bool vtkVgAppendImageData::Finish()
{
  if (this->WorkerThreadId != -1
    && this->Threader->IsThreadActive(this->WorkerThreadId))
    {
    // Make sure that you wait for the thread to finish.
    this->ConditionLock->Lock();
    this->Condition->Wait(this->ConditionLock);
    this->ConditionLock->Unlock();

    this->Threader->TerminateThread(this->WorkerThreadId);
    }

  if (this->Threader->IsThreadActive(this->WorkerThreadId))
    {
    return false;
    }
  else
    {
    return true;
    }
}

//-----------------------------------------------------------------------------
bool vtkVgAppendImageData::Kill()
{
  // Kill the thread forecefully.
  if (this->WorkerThreadId != -1
    && this->Threader->IsThreadActive(this->WorkerThreadId))
    {
    this->Threader->TerminateThread(this->WorkerThreadId);
    }

  if (this->Threader->IsThreadActive(this->WorkerThreadId))
    {
    return false;
    }
  else
    {
    return true;
    }
}

//-----------------------------------------------------------------------------
void vtkVgAppendImageData::ComputerInternalFileName()
{
  std::string fileName(this->BaseFileName);

//  std::cout << "fileName " << fileName << std::endl;

  size_t posUS;
  size_t posDot;

  posUS = fileName.find_last_of("_");

  if (posUS == std::string::npos)
    {
    // Throw an error.
    return;
    }

//  std::cout << "posUS" << posUS << std::endl;

  posDot = fileName.find_last_of(".");
  if (posDot == std::string::npos)
    {
    // Throw an error.
    return;
    }

//  std::cout << "posDot" << posDot << std::endl;

  std::string prefix = fileName.substr(0, posUS + 1);
  this->SetBaseFileNamePrefix(prefix.c_str());
//  std::cout << "BaseFileNamePrefix: " << this->BaseFileNamePrefix << std::endl;

  std::string numStr = fileName.substr(posUS + 1, posDot - (posUS + 1));
//  std::cout << "numStr is: " << numStr << std::endl;

  std::istringstream iss;
  iss.str(numStr);
  iss >> this->BaseNum;

//  std::cout << "Check: this->BaseNum " << this->BaseNum << std::endl;

  this->SetBaseFileNameNum(numStr.c_str());
}

void vtkVgAppendImageData::Init()
{
  std::string ext
    = vtksys::SystemTools::GetFilenameExtension(this->BaseFileName);
  if (ext.compare(".mrj"))
    {
    this->VGImageReader = vtkVgMultiResJpgImageReader2::New();
    }
  else if (ext.compare(".jp2"))
    {
    this->VGImageReader = vtkVgImageSource::New();
    }
  else
    {
    }

  this->VGImageReader->SetFileName(this->BaseFileName);
  this->VGImageReader->SetUseOutputResolution(this->UseOutputResolution);
  this->VGImageReader->SetReadExtents(this->ReadExtents);
  this->VGImageReader->SetOutputResolution(this->OutputResolution[0],
                                           this->OutputResolution[1]);
  this->VGImageReader->Update();
}
