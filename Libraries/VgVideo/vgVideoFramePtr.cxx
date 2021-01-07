// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
