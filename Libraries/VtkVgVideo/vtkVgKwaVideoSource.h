/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgKwaVideoSource_h
#define __vtkVgKwaVideoSource_h

#include <vtkObject.h>

#include <vtkVgVideoSourceBase.h>

class VTKVG_VIDEO_EXPORT vtkVgKwaVideoSource : public vtkVgVideoSourceBase
{
public:
  static vtkVgKwaVideoSource* New();
  vtkTypeMacro(vtkVgKwaVideoSource, vtkVgVideoSourceBase);

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Open a video.
  bool Open(const char* uri);

  // \copydoc vtkVgVideoSourceBase::ResolveSeek
  virtual vtkVgTimeStamp ResolveSeek(
    const vtkVgTimeStamp&,
    vg::SeekMode direction = vg::SeekNearest) const;

  // \copydoc vtkVgVideoSourceBase::GetFrame
  virtual vtkVgVideoFrame GetFrame(
    const vtkVgTimeStamp&,
    vg::SeekMode direction = vg::SeekNearest) const;

  // \copydoc vtkVgVideoSourceBase::GetMetadata
  virtual vtkVgVideoFrameMetaData GetMetadata(
    const vtkVgTimeStamp&,
    vg::SeekMode direction = vg::SeekNearest) const;

  // \copydoc vtkVgVideoSourceBase::GetMinTime
  virtual vtkVgTimeStamp GetMinTime() const;

  // \copydoc vtkVgVideoSourceBase::GetMaxTime
  virtual vtkVgTimeStamp GetMaxTime() const;

  // \copydoc vtkVgVideoSourceBase::GetFrameCount
  virtual int GetFrameCount() const;

protected:
  vtkVgKwaVideoSource();
  virtual ~vtkVgKwaVideoSource();

private:
  class vtkInternal;
  const vtkInternal* Internal;

  vtkVgKwaVideoSource(const vtkVgKwaVideoSource&); // disabled
  vtkVgKwaVideoSource& operator=(const vtkVgKwaVideoSource&); // disabled
};

#endif
