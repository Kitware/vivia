/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// Qt includes
#include <QFileInfo>
#include <QHash>
#include <QMap>
#include <QMultiHash>
#include <QScopedPointer>

// QtExtensions includes
#include <qtUtil.h>

// visgui includes
#include <vtkVgAdapt.h>
#include <vtkVgCompositeEventRepresentation.h>
#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>
#include <vtkVgVideoModel0.h>
#include <vtkVgVideoNode.h>

// VTK includes
#include <vtkIdList.h>
#include <vtkPoints.h>

#include <vvClipVideoRepresentation.h>

// viqui includes
#include "vqArchiveVideoSource.h"
#include "vqQueryVideoPlayer.h"
#include "vqSettings.h"

#include <vvQueryVideoPlayerPrivate.h>

struct vqQueryVideoPlayer::qvpInternal
{
  qvpInternal() :
    StartTime(-1),
    EndTime(-1)
    {
    }

  vtkSmartPointer<vqArchiveVideoSource> VideoSource;

  double StartTime;
  double EndTime;
};

//-----------------------------------------------------------------------------
vqQueryVideoPlayer::vqQueryVideoPlayer(QWidget* parent)
  : vvQueryVideoPlayer(parent), MyInternal(new qvpInternal)
{
}

//-----------------------------------------------------------------------------
vqQueryVideoPlayer::~vqQueryVideoPlayer()
{
  delete MyInternal;
}

//-----------------------------------------------------------------------------
bool vqQueryVideoPlayer::setVideoUri(QUrl uri)
{
  if (uri.isEmpty())
    {
    // Clear video source
    this->MyInternal->VideoSource = 0;
    return true;
    }

  // Find archive files associated with this video
  QFileInfo vfi(uri.encodedPath());
  QUrl archiveUri = vqSettings().queryVideoUri();
  archiveUri.setEncodedPath(archiveUri.encodedPath() + "/"
                            + vfi.completeBaseName().toAscii() + ".index");

  this->MyInternal->VideoSource =
    vtkSmartPointer<vqArchiveVideoSource>::New();
  this->MyInternal->VideoSource->SetMissionId("");
  this->MyInternal->VideoSource->SetStreamId("");
  this->MyInternal->VideoSource->SetTimeRange(this->MyInternal->StartTime,
                                              this->MyInternal->EndTime);
  if (this->MyInternal->VideoSource->AcquireVideoClip(archiveUri) != VTK_OK)
    {
    // Not successful
    this->MyInternal->VideoSource = 0;
    return false;
    }

  // Load was successful; return archive
  return true;
}

//-----------------------------------------------------------------------------
void vqQueryVideoPlayer::buildVideoModel()
{
  this->Internal->VideoModel = vtkSmartPointer<vtkVgVideoModel0>::New();
  this->Internal->VideoModel->SetVideoSource(this->MyInternal->VideoSource);
  this->Internal->VideoModel->SetUseInternalTimeStamp(1);
  this->Internal->VideoModel->SetLooping(1);
  this->Internal->VideoModel->SetId(0);
}

//-----------------------------------------------------------------------------
int vqQueryVideoPlayer::videoHeight()
{
  static const int bestGuess = 480;
  if (!this->Internal->VideoModel)
    {
    return bestGuess;
    }

  return static_cast<vqArchiveVideoSource*>(
           this->Internal->VideoModel->GetVideoSource())->GetVideoHeight();
}

//-----------------------------------------------------------------------------
void vqQueryVideoPlayer::setTimeRange(double startTime, double endTime)
{
  this->MyInternal->StartTime = startTime;
  this->MyInternal->EndTime = endTime;
}

//-----------------------------------------------------------------------------
void vqQueryVideoPlayer::reset()
{
  this->MyInternal->VideoSource = 0;
  vvQueryVideoPlayer::reset();
}
