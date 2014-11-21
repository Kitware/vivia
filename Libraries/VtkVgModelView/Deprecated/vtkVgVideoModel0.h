/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgVideoModel0_h
#define __vtkVgVideoModel0_h

// Single entity that represents a video with tracks, events, and other
// items.

// VG includes.
#include "vtkVgModelBase.h"
#include "vtkVgVideoFrameData.h"

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

// Forward declarations.
class vtkVgBaseImageSource;
class vtkVgEventModel;
class vtkVgTrackModel;
class vtkVgVideoProviderBase;

class vtkImageData;
class vtkMatrix4x4;
class vtkTimerLog;

class VTKVG_MODELVIEW_EXPORT vtkVgVideoModel0 : public vtkVgModelBase
{
public:
  // Description:
  // Using Macro for convenience
  vtkVgClassMacro(vtkVgVideoModel0);

  // Description:
  // Use \c New() to create instance of \c vtkVgVideoModel0
  static vtkVgVideoModel0* New();

  // Description:
  // Using VTK macro for convenience
  vtkTypeMacro(vtkVgVideoModel0, vtkVgModelBase);

  // Description:
  // Serialize various states
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set \c UseFrameIndex to ( 1 ), if frame inden should be used
  // when using timestamp to retrive frame data or ( 0 ), if time
  // should be used instead.
  // \sa UseTimeStamp
  vtkSetMacro(UseFrameIndex, int);
  vtkBooleanMacro(UseFrameIndex, int);

  // Description:
  // Return whether \UseFrameIndex is enabled ( 1 ) or disabled ( 0 )
  vtkGetMacro(UseFrameIndex, int);

  // Description:
  // Set \c Looping to ( 1 ) or ( 0 ) to enable and disable
  // looping at any given time.
  // Default is ( 0 )
  vtkSetMacro(Looping, int);
  vtkBooleanMacro(Looping, int);

  // Description:
  // Return whether Looping is enabled ( 1 ) or disabled ( 0 )
  vtkGetMacro(Looping, int);

  // Description:
  // Set if video needs to played from beginning or from padded offset
  vtkSetMacro(PlayFromBeginning, int);
  vtkGetMacro(PlayFromBeginning, int);
  vtkBooleanMacro(PlayFromBeginning, int);

  // Description:
  // Set/Get \c vtkVgVideoProviderBase.
  void SetVideoSource(vtkVgVideoProviderBase* videoSource);
  vtkVgVideoProviderBase* GetVideoSource();
  const vtkVgVideoProviderBase* GetVideoSource() const;

  // Description:
  virtual int Update(const vtkVgTimeStamp& timeStamp,
                     const vtkVgTimeStamp* referenceFrameTimeStamp);
  using Superclass::Update;

  // Description:
  // Method for playback controls
  virtual int Play();
  virtual int IsPlaying();

  virtual int Pause();
  virtual int IsPaused();

  virtual int Stop();
  virtual int IsStopped();

  int Next();
  int Previous();

  int SeekTo(double time);

  // Description:
  // Return raw data related to a frame
  const vtkVgVideoFrameData* GetFrameData();

  // Description:
  // Set homogeneous transformation matrix for the video
  void SetVideoMatrix(vtkMatrix4x4* matrix);

  void SetEventModel(vtkVgEventModel* eventModel);
  vtkGetObjectMacro(EventModel, vtkVgEventModel);

  void SetTrackModel(vtkVgTrackModel* trackModel);
  vtkGetObjectMacro(TrackModel, vtkVgTrackModel);

  // Description:
  // Return the MTime for the last Update (that did something).
  virtual unsigned long GetUpdateTime();

  // Description:
  // Set \UseTimeStamp to ( 1 ), if timestamp should be used to
  // fetch frame from a source or ( 0 ), if next or previous
  // frame in sequence should be fetched.
  vtkSetMacro(UseTimeStamp, int);
  vtkGetMacro(UseTimeStamp, int);

  // Description:
  vtkSetMacro(PlaybackSpeed, double);
  vtkGetMacro(PlaybackSpeed, double);

  // Description:
  // Return current seek time. \c CurrentSeekTime is
  // used when playing video in real time mode.
  inline double GetCurrentSeekTime() const
    {
    return this->CurrentSeekTime;
    }


protected:

  // \copydoc vtkVgModelBase::SetInitialized
  virtual void SetInitialized(int arg);

  int ActionPlay(const vtkVgTimeStamp& timeStamp,
                 const vtkVgTimeStamp* referenceFrameTimeStamp = 0);
  int ActionPause(const vtkVgTimeStamp& timeStamp,
                  const vtkVgTimeStamp* referenceFrameTimeStamp = 0);
  int ActionStop(const vtkVgTimeStamp& timeStamp,
                 const vtkVgTimeStamp* referenceFrameTimeStamp = 0);

  void OnActionPlaySuccess(const vtkVgTimeStamp& timeStamp,
                           const vtkVgTimeStamp* referenceFrameTimeStamp = 0);
  void OnActionPlayFailure(const vtkVgTimeStamp& timeStamp,
                           const vtkVgTimeStamp* referenceFrameTimeStamp = 0);

  bool SkipPadding(bool updateSeekTo = true);
  bool UndoSkipPadding(bool updateSeekTo = true);

  int GetFrameUsingTimeStamp(const vtkVgTimeStamp& timeStamp);
  int GetSequencialFrame();

  typedef vtkSmartPointer<vtkImageData>   vtkImageDataRef;

  vtkVgVideoModel0();
  virtual ~vtkVgVideoModel0();

  // Description:
  // Local time / global time?
  // WE MAY NEED BEGIN / END TIME FOR THIS VIDEO ITEM.

  // Description:
  int                   NumberOfFrames;

  // Description:
  // Ratio of width of camera to viewport width.
  double                VisibleScale;

  // Description:
  // Visible extents in object space.
  vtkIdType             VisibleExtents[6];

  // Description:
  // States.
  bool                  UseSourceTimeStamp;

  int                   Playing;
  int                   Paused;
  int                   Stopped;

  int                   Looping;
  int                   LastLoopingState;

  // Description:
  // Use index or time?
  int                   UseFrameIndex;

  // Description:
  // In future we may have an even higher level abstract
  // source.
  vtkSmartPointer<vtkVgVideoProviderBase>
  VideoSource;

  // Description:
  // Data per frame for a video clip
  vtkVgVideoFrameData*   VideoFrameData;

  // Description:
  // The EventModel for this clip
  vtkVgEventModel*       EventModel;
  vtkVgTrackModel*       TrackModel;

  // Description:
  // MTime for the last Update (that did something)
  vtkTimeStamp          UpdateTime;

  // Description:
  // Flag used to set the behavior of play. If set to true
  // video will be played from first frame or else it will be played
  // starting from its current position.
  int                   PlayFromBeginning;

  // Description:
  // Flag that determines if timestamp should be used to fetch
  // frame data.
  // Default is \c true.
  int                   UseTimeStamp;

  // Description:
  // Default is 1.0
  double                PlaybackSpeed;

  // Description:
  // Current seek time. \c CurrenSeekTime is used for realtime mode
  // or when \c UseInternalTimeStamp is set to \c true.
  double                CurrentSeekTime;

  // Description:
  // Variables that are used for tracking application time
  double                LastAppTime;
  double                CurrentAppTime;

  // Description:
  // Cache start and end time of the clip. This is used when playing
  // in real time mode or when \c UseInternalTimeStamp is set to true.
  double                ClipTimeRangeCache[2];

private:
  vtkVgVideoModel0(const vtkVgVideoModel0&);  // Not implemented.
  void operator=(const vtkVgVideoModel0&);   // Not implemented.
};

#endif
