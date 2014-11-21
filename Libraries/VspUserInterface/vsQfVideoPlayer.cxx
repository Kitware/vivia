/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsQfVideoPlayer.h"

#include <vvQueryVideoPlayerPrivate.h>

#include "vsVideoPlayerModel.h"

// TODO: Why does this include need to come last on MSVC?
#include "vsQfVideoSource.h"

//-----------------------------------------------------------------------------
vsQfVideoPlayer::vsQfVideoPlayer(vsQfVideoSource* source, QWidget* parent)
  : vvQueryVideoPlayer(parent), Source(source)
{
}

//-----------------------------------------------------------------------------
vsQfVideoPlayer::~vsQfVideoPlayer()
{
}

//-----------------------------------------------------------------------------
void vsQfVideoPlayer::buildVideoModel()
{
  vsVideoPlayerModel* model = vsVideoPlayerModel::New();

  model->SetVideoSource(this->Source);
  model->SetUseInternalTimeStamp(1);
  model->SetSharingSource(1);
  model->SetLooping(1);
  model->SetId(0);

  this->Internal->VideoModel = model;
  model->Delete();

  connect(this->Source,
          SIGNAL(frameAvailable()),
          model,
          SLOT(OnFrameAvailable()));

  connect(model,
          SIGNAL(FrameAvailable()),
          this,
          SLOT(onFrameAvailable()));
}

//-----------------------------------------------------------------------------
int vsQfVideoPlayer::videoHeight()
{
  // TODO
  return 480;
}

//-----------------------------------------------------------------------------
void vsQfVideoPlayer::reset()
{
  // TODO
  vvQueryVideoPlayer::reset();
}
