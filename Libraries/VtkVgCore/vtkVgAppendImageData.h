// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgAppendImageData_h
#define __vtkVgAppendImageData_h

#include "vtkImageAlgorithm.h"

#include <vgExport.h>

// Forward declarations.
class vtkAppendImageDataInternal;
class vtkImageReader2;
class vtkVgBaseImageSource;
class vtkMutexLock;
class vtkMultiThreader;
class vtkConditionVariable;

class VTKVG_CORE_EXPORT vtkVgAppendImageData : public vtkImageAlgorithm
{
public:
  static vtkVgAppendImageData* New();
  vtkTypeMacro(vtkVgAppendImageData, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Get/Set the first file name in the list of filenames.
  // This should include the path as well for ex. on Unix
  // systems it would be /home/...../test_00001.jp2. Naming
  // convention is important here as anything after last '_'
  // will be considered as the index.
  vtkGetStringMacro(BaseFileName);
  vtkSetStringMacro(BaseFileName);

  // Get/Set the number of files to be read including the
  // \c BaseFileName.
  vtkGetMacro(NoOfFiles, int);
  vtkSetMacro(NoOfFiles, int);

  // Get/Set the flag to use requested resolution. If set to false
  // image won't be sampled.
  vtkGetMacro(UseOutputResolution, bool);
  vtkSetMacro(UseOutputResolution, bool);
  vtkBooleanMacro(UseOutputResolution, bool);

  // Get/Set requested resolution. If \c UseOutputResolution
  // set to true and given these values the image will sampled
  // accordingly. For example if the reader reads the entire image
  // of 1k X 1k size and user has set w, h = 500 then image will
  // be downsampled.
  void GetOutputResolution(int& w, int& h) const;
  void SetOutputResolution(int w, int h);

  // Get/Set the extents of the data to be read. Currently there is
  // no check if this extent is bigger than the whole extent of the
  // data.
  vtkSetVector4Macro(ReadExtents, int);
  vtkGetVector4Macro(ReadExtents, int);

  // Use this function to query what slices has been read. Passed
  // array should have enough memory to held \c NoOfFiles flags.
  void UpdateStateInfo(int* stArr);

  // Thread related functions.
  void        Start();
  void        Stop();
  bool        Finish();
  bool        Kill();

  // This is not required as of now.
//  vtkImageData*        GetData();

protected:

  vtkVgAppendImageData();
  virtual ~vtkVgAppendImageData();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  // Useful functions.
  void        ComputerInternalFileName();

  // This function is needed to that we can query some meta data.
  void        Init();

  vtkSetStringMacro(BaseFileNamePrefix);
  vtkSetStringMacro(BaseFileNameNum);

// Data:
protected:

  bool  StopThread;
  bool  UseOutputResolution;

  char* BaseFileName;
  char* BaseFileNamePrefix;
  char* BaseFileNameNum;

  int   NoOfFiles;
  int   BaseNum;
  int   WorkerThreadId;
  int*  StateArray;

  int   OutputResolution[2];
  int   ReadExtents[4];

  vtkVgBaseImageSource* VGImageReader;

  vtkMultiThreader* Threader;

  // Lock used for read / write StateArray.
  vtkMutexLock*     ProcessingLock;

  // Lock and condition used for making sure
  // application wait for the thread to finish
  // unless application calls \c Stop().
  vtkMutexLock*         ConditionLock;
  vtkConditionVariable* Condition;

  // Lock used for read / write StopThread.
  vtkMutexLock*         Lock;

  vtkAppendImageDataInternal* Implementation;

private:
  vtkVgAppendImageData(const vtkVgAppendImageData&);  // Not implemented.
  void operator=(const vtkVgAppendImageData&);      // Not implemented.
};

#endif // __vtkVgAppendImageData_h
