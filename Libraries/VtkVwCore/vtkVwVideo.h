// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVwVideo_h
#define __vtkVwVideo_h

#include <vtkObject.h>

#include <vtkVgTimeStamp.h>
#include <vgNamespace.h>

class VTKVW_CORE_EXPORT vtkVwVideo : public vtkObject
{
public:
  static vtkVwVideo* New();
  vtkTypeMacro(vtkVwVideo, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Open a video.
  bool Open(const char* uri);

  // Description:
  // Get a frame from the video.
  std::string GetFrame(const char* jsReferenceTime, int direction) const;

  // Description:
  // Get the first time stamp available in the video.
  std::string GetFirstTime() const;

  // Description:
  // Get the last time stamp available in the video.
  std::string GetLastTime() const;

  // Description:
  // Get the number of frames available in the video clip.
  int GetFrameCount() const;

protected:
  vtkVwVideo();
  virtual ~vtkVwVideo();

private:
  class vtkInternal;
  const vtkInternal* Internal;

  vtkVwVideo(const vtkVwVideo&); // disabled
  vtkVwVideo& operator=(const vtkVwVideo&); // disabled
};

#endif
