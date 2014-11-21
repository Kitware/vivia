/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgVideoSourceBase_h
#define __vtkVgVideoSourceBase_h

#include <vtkObject.h>
#include <vtkSmartPointer.h>

#include <vgExport.h>

#include <vgNamespace.h>

#include "vtkVgMacros.h"

class  vtkVgTimeStamp;
struct vtkVgVideoFrame;
struct vtkVgVideoFrameMetaData;

class VTKVG_CORE_EXPORT vtkVgVideoSourceBase : public vtkObject
{
public:
  vtkVgClassMacro(vtkVgVideoSourceBase);
  vtkTypeMacro(vtkVgVideoSourceBase, vtkObject);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get video time stamp matching seek request.
  //
  // This is intended to allow users to look up the time stamp that a frame
  // request would give, in order to test if they already have that time stamp,
  // in order to avoid the overhead of retrieving the actual pixels.
  virtual vtkVgTimeStamp ResolveSeek(
    const vtkVgTimeStamp&,
    vg::SeekMode direction = vg::SeekNearest) const = 0;

  // Description:
  // Get a frame from the video.
  virtual vtkVgVideoFrame GetFrame(
    const vtkVgTimeStamp&,
    vg::SeekMode direction = vg::SeekNearest) const = 0;

  // Description:
  // Get metadata for the specified frame.
  virtual vtkVgVideoFrameMetaData GetMetadata(
    const vtkVgTimeStamp&,
    vg::SeekMode direction = vg::SeekNearest) const = 0;

  // Description:
  // Get the temporally 'smallest' time stamp available in the video.
  virtual vtkVgTimeStamp GetMinTime() const = 0;

  // Description:
  // Get the temporally 'largest' time stamp available in the video.
  virtual vtkVgTimeStamp GetMaxTime() const = 0;

  // Description:
  // Get the number of frames available in the video clip.
  virtual int GetFrameCount() const = 0;

protected:
  vtkVgVideoSourceBase();
  virtual ~vtkVgVideoSourceBase();

private:
  vtkVgVideoSourceBase(const vtkVgVideoSourceBase&); // disabled
  vtkVgVideoSourceBase& operator=(const vtkVgVideoSourceBase&); // disabled
};

#endif // __vtkVgVideoSourceBase_h
