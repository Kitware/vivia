// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
