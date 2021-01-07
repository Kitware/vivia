// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvVideoPlayerPrivate_h
#define __vvVideoPlayerPrivate_h

#include <QScopedPointer>

#include <vtkSmartPointer.h>

#include <vgExport.h>

#include <vtkVgTimeStamp.h>

#include "ui_videoView.h"

class QTimer;

class qtUiState;

class vtkVgVideoNode;
class vtkVgVideoViewer;

class vtkTimerLog;

class VV_VTKWIDGETS_EXPORT vvVideoPlayerPrivate
{
public:
  vvVideoPlayerPrivate();
  virtual ~vvVideoPlayerPrivate();

  Ui::videoView UI;
  QScopedPointer<qtUiState> UiState;

  vtkSmartPointer<vtkVgVideoViewer> Viewer;
  vtkSmartPointer<vtkVgVideoNode> CurrentVideo;
  bool IsExternal;

  vtkVgTimeStamp TimeStamp;
  double CurrentTime;

  bool AutoZoomEnabled;
  bool AllowPicking;
  int ResetView;

  vtkSmartPointer<vtkVgVideoNode> CurrentVideoPicked;

  double FrameExtents[4];
  double CurrentExtents[4];
  double CurrentCenter[3];

  bool HasUpdated;
  bool UpdatingFrameScrubber;
  bool Seeking;

  QScopedPointer<QTimer> UpdateTimer;
  int UpdateIntervalMSec;

  double PlaybackSpeed;

  vtkSmartPointer<vtkTimerLog> TimerLog;
  double PlayerInitTime;
};

#endif // __vvVideoPlayer_h
