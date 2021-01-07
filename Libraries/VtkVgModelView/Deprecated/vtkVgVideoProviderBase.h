// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoProviderBase_h
#define __vtkVgVideoProviderBase_h

#include "vtkVgDataSourceBase.h"

// VTK includes.
#include <vtkSmartPointer.h>

// C++ includes
#include <map>
#include <utility>
#include <vector>

#include <vgExport.h>

class  vtkVgTimeStamp;
struct vtkVgVideoFrameData;
struct vtkVgVideoMetadata;

class VTKVG_MODELVIEW_EXPORT vtkVgVideoProviderBase : public vtkVgDataSourceBase
{
public:
  // Description:
  // Easy to use types.
  vtkVgClassMacro(vtkVgVideoProviderBase);

  vtkTypeMacro(vtkVgVideoProviderBase, vtkVgDataSourceBase);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the time range for this source if there is any.
  vtkSetVector2Macro(TimeRange, double);
  vtkGetVector2Macro(TimeRange, double);

  // Description:
  // Set/Get time interval.
  vtkSetMacro(TimeInterval, double);
  vtkGetMacro(TimeInterval, double);

  // Description:
  // Set/Get frame range.
  vtkSetVector2Macro(FrameRange, int);
  vtkGetVector2Macro(FrameRange, int);

  // Description:
  // Set/Get frame interval.
  vtkSetMacro(FrameInterval, int);
  vtkGetMacro(FrameInterval, int);

  // Decription:
  // Set/Get the padding requested on either end of the clip (defaults to 0)
  vtkSetMacro(RequestedPadding, double);
  vtkGetMacro(RequestedPadding, double);

  // Description:
  // Toggle looping.
  vtkSetMacro(Looping, int);
  vtkGetMacro(Looping, int);
  vtkBooleanMacro(Looping, int);

  // Description:
  // Set / Get mission id to be used when retrieving video.
  vtkSetStringMacro(MissionId);
  vtkGetStringMacro(MissionId);

  // Description:
  // Set / Get stream id to be used when retrieving video.
  vtkSetStringMacro(StreamId);
  vtkGetStringMacro(StreamId);

  // Description:
  // Get the next frame based on time or frame number.
  virtual int GetFrame(vtkVgVideoFrameData* frameData, double time)  = 0;
  virtual int GetFrame(vtkVgVideoFrameData* frameData, int frameNumber) = 0;

  // Description:
  // Get the next frame regardless of time or frame number.
  virtual int GetNextFrame(vtkVgVideoFrameData* frameData) = 0;

  // Description:
  // Get current frame.
  virtual int GetCurrentFrame(vtkVgVideoFrameData* frameData) = 0;

  // Description:
  // Get the previous frame regardless of time or frame number.
  virtual int GetPreviousFrame(vtkVgVideoFrameData* frameData) = 0;

  // Description:
  // Return total number of frames. A value of lower than 0 could mean
  // that this source either streams the frames or its unable
  // to calculate number of frames.
  virtual int GetNumberOfFrames() = 0;

  // Description:
  // Get a map of all currently available metadata.
  virtual std::map<vtkVgTimeStamp, vtkVgVideoMetadata> GetMetadata() = 0;

  // Description:
  // Get metadata for the current frame.
  virtual int GetCurrentMetadata(vtkVgVideoMetadata* metadata) = 0;

  // Description:
  // Reset to beginning of frame.
  // This won't work if you are using time stamps.
  virtual int Reset() = 0;

  // Description:
  // Advance the clip
  virtual bool Advance() = 0;

  // Description:
  // Recede the clip
  virtual bool Recede() = 0;

  // Description:
  // Position the internal frame pointer such that the next call to
  // GetNextFrame will retrieve a frame with timestamp the same as \a
  // time, or as close as possible earlier than \a time.
  // Return false if \a time is out of the range of the clip.
  virtual bool SeekNearestEarlier(double time) = 0;

  virtual void ShallowCopy(vtkVgDataSourceBase& other);
  virtual void DeepCopy(vtkVgDataSourceBase& other);

  void AddTimeMark(double startTime, double endTime);
  std::vector<std::pair<double, double> > GetTimeMarks();

protected:
  vtkVgVideoProviderBase();
  virtual ~vtkVgVideoProviderBase();

  typedef std::pair<double, double> TimeMark;
  std::vector<TimeMark> TimeMarks;

  double TimeRange[2];
  double TimeInterval;
  double RequestedPadding;

  int    FrameRange[2];
  int    FrameInterval;

  int    Looping;

  char* MissionId;
  char* StreamId;

private:
  vtkVgVideoProviderBase(const vtkVgVideoProviderBase&);  // Not implemented.
  void operator= (const vtkVgVideoProviderBase&);       // Not implemented.
};

#endif // __vtkVgVideoProviderBase_h
