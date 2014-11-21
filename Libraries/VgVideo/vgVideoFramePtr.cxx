/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgVideoFramePtrPrivate.h"

QTE_IMPLEMENT_D_FUNC_CONST(vgVideoFramePtr)

//-----------------------------------------------------------------------------
vgVideoFramePtr::vgVideoFramePtr()
{
}

//-----------------------------------------------------------------------------
vgVideoFramePtr::vgVideoFramePtr(const vgVideoFramePtr& other) :
  d_ptr(other.d_ptr)
{
}

//-----------------------------------------------------------------------------
vgVideoFramePtr::vgVideoFramePtr(vgVideoFramePtrPrivate* d) : d_ptr(d)
{
}

//-----------------------------------------------------------------------------
vgVideoFramePtr::~vgVideoFramePtr()
{
}

//-----------------------------------------------------------------------------
vgVideoFramePtr& vgVideoFramePtr::operator=(const vgVideoFramePtr& other)
{
  this->d_ptr = other.d_ptr;
  return *this;
}

//-----------------------------------------------------------------------------
bool vgVideoFramePtr::isValid() const
{
  QTE_D_CONST(vgVideoFramePtr);
  return d;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgVideoFramePtr::time() const
{
  QTE_D_CONST(vgVideoFramePtr);
  return (d ? d->time : vgTimeStamp());
}

//-----------------------------------------------------------------------------
vgImage vgVideoFramePtr::image() const
{
  QTE_D_CONST(vgVideoFramePtr);
  return (d ? d->data() : vgImage());
}

//-----------------------------------------------------------------------------
vgImage vgVideoFramePtr::operator*() const
{
  return this->image();
}
