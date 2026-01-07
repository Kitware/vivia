// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsStaticSourceFactory.h"

#include <QList>

// Need these so that QSharedPointer can find their dtors
#include "vsVideoSource.h"
#include "vsTrackSource.h"
#include "vsDescriptorSource.h"

//-----------------------------------------------------------------------------
class vsStaticSourceFactoryPrivate
{
public:
  vsVideoSourcePtr VideoSource;
  QList<vsTrackSourcePtr> TrackSources;
  QList<vsDescriptorSourcePtr> DescriptorSources;
};

QTE_IMPLEMENT_D_FUNC(vsStaticSourceFactory)

//-----------------------------------------------------------------------------
vsStaticSourceFactory::vsStaticSourceFactory() :
  d_ptr(new vsStaticSourceFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
vsStaticSourceFactory::~vsStaticSourceFactory()
{
}

//-----------------------------------------------------------------------------
vsVideoSourcePtr vsStaticSourceFactory::videoSource() const
{
  QTE_D_CONST(vsStaticSourceFactory);
  return d->VideoSource;
}

//-----------------------------------------------------------------------------
QList<vsTrackSourcePtr> vsStaticSourceFactory::trackSources() const
{
  QTE_D_CONST(vsStaticSourceFactory);
  return d->TrackSources;
}

//-----------------------------------------------------------------------------
QList<vsDescriptorSourcePtr> vsStaticSourceFactory::descriptorSources() const
{
  QTE_D_CONST(vsStaticSourceFactory);
  return d->DescriptorSources;
}

//-----------------------------------------------------------------------------
void vsStaticSourceFactory::setVideoSource(const vsVideoSourcePtr& sourcePtr)
{
  QTE_D(vsStaticSourceFactory);
  d->VideoSource = sourcePtr;
}

//-----------------------------------------------------------------------------
void vsStaticSourceFactory::setVideoSource(vsVideoSource* source)
{
  this->setVideoSource(vsVideoSourcePtr(source));
}

//-----------------------------------------------------------------------------
void vsStaticSourceFactory::addTrackSource(const vsTrackSourcePtr& sourcePtr)
{
  QTE_D(vsStaticSourceFactory);
  d->TrackSources.append(sourcePtr);
}

//-----------------------------------------------------------------------------
void vsStaticSourceFactory::addTrackSource(vsTrackSource* source)
{
  this->addTrackSource(vsTrackSourcePtr(source));
}

//-----------------------------------------------------------------------------
void vsStaticSourceFactory::addDescriptorSource(
  const vsDescriptorSourcePtr& sourcePtr)
{
  QTE_D(vsStaticSourceFactory);
  d->DescriptorSources.append(sourcePtr);
}

//-----------------------------------------------------------------------------
void vsStaticSourceFactory::addDescriptorSource(vsDescriptorSource* source)
{
  this->addDescriptorSource(vsDescriptorSourcePtr(source));
}
