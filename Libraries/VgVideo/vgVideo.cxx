/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgVideo.h"
#include "vgVideoPrivate.h"

QTE_IMPLEMENT_D_FUNC(vgVideo)

//-----------------------------------------------------------------------------
vgVideoPrivate::vgVideoPrivate()
{
}

//-----------------------------------------------------------------------------
vgVideoPrivate::~vgVideoPrivate()
{
}

//-----------------------------------------------------------------------------
vgVideo::vgVideo(vgVideoPrivate* d) : d_ptr(d)
{
  // Reset iterator
  this->rewind();
}

//-----------------------------------------------------------------------------
vgVideo::~vgVideo()
{
}

//-----------------------------------------------------------------------------
vgVideoFramePtr vgVideo::currentFrame() const
{
  QTE_D_CONST(vgVideo);
  if (d->pos == d->frames.end())
    {
    return vgVideoFramePtr();
    }
  return d->pos.value();
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideo::currentTimeStamp() const
{
  QTE_D_CONST(vgVideo);
  if (d->pos == d->frames.end())
    {
    return vgTimeStamp();
    }
  return d->pos.key();
}

//-----------------------------------------------------------------------------
vgImage vgVideo::currentImage() const
{
  QTE_D_CONST(vgVideo);
  if (d->pos == d->frames.end())
    {
    return vgImage();
    }
  return *d->pos.value();
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideo::advance()
{
  QTE_D(vgVideo);
  if (d->pos == d->frames.end())
    {
    return vgTimeStamp();
    }

  ++d->pos;
  if (d->pos == d->frames.end())
    {
    return vgTimeStamp();
    }

  return d->pos.key();
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideo::recede()
{
  QTE_D(vgVideo);
  if (d->pos == d->frames.begin())
    {
    return vgTimeStamp();
    }

  --d->pos;
  return d->pos.key();
}

//-----------------------------------------------------------------------------
void vgVideo::rewind()
{
  QTE_D(vgVideo);
  d->pos = d->frames.begin();
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideo::seek(vgTimeStamp pos, vg::SeekMode direction)
{
  QTE_D(vgVideo);
  d->pos = this->iterAt(pos, direction);
  return this->currentTimeStamp();
}

//-----------------------------------------------------------------------------
vgVideoFramePtr vgVideo::frameAt(
  vgTimeStamp pos, vg::SeekMode direction) const
{
  QTE_D_CONST(vgVideo);
  FrameMap::const_iterator iter = this->iterAt(pos, direction);
  if (iter == d->frames.end())
    {
    return vgVideoFramePtr();
    }
  return iter.value();
}

//-----------------------------------------------------------------------------
vgImage vgVideo::imageAt(vgTimeStamp pos, vg::SeekMode direction) const
{
  QTE_D_CONST(vgVideo);
  FrameMap::const_iterator iter = this->iterAt(pos, direction);
  if (iter == d->frames.end())
    {
    return vgImage();
    }
  return *iter.value();
}

//-----------------------------------------------------------------------------
vgVideo::FrameMap vgVideo::frames() const
{
  QTE_D_CONST(vgVideo);
  return d->frames;
}

//-----------------------------------------------------------------------------
vgRange<vgTimeStamp> vgVideo::timeRange() const
{
  QTE_D_CONST(vgVideo);
  if (d->frames.count() < 1)
    {
    return vgRange<vgTimeStamp>();
    }
  return vgRange<vgTimeStamp>(d->frames.begin().key(),
                              (--d->frames.end()).key());
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideo::firstTime() const
{
  QTE_D_CONST(vgVideo);
  if (d->frames.count() < 1)
    {
    return vgTimeStamp();
    }
  return d->frames.begin().key();
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideo::lastTime() const
{
  QTE_D_CONST(vgVideo);
  if (d->frames.count() < 1)
    {
    return vgTimeStamp();
    }
  return (--d->frames.end()).key();
}

//-----------------------------------------------------------------------------
int vgVideo::frameCount() const
{
  QTE_D_CONST(vgVideo);
  return d->frames.count();
}

//-----------------------------------------------------------------------------
vgVideo::FrameMap::const_iterator vgVideo::iterAt(
  vgTimeStamp pos, vg::SeekMode direction) const
{
  QTE_D_CONST(vgVideo);
  return d->frames.find(pos, direction);
}
