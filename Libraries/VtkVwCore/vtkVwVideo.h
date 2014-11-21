/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
