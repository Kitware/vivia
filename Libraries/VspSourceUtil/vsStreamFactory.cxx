// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsStreamFactory.h"

#include "vsStreamSource.h"

// Need these so that QSharedPointer can find their dtors
#include <vsDescriptorSource.h>
#include <vsTrackSource.h>

QTE_IMPLEMENT_D_FUNC(vsStreamFactory)

//-----------------------------------------------------------------------------
class vsStreamFactoryPrivate
{
public:
  QSharedPointer<vsStreamSource> source;
};

//-----------------------------------------------------------------------------
vsStreamFactory::vsStreamFactory() : d_ptr(new vsStreamFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
vsStreamFactory::~vsStreamFactory()
{
}

//-----------------------------------------------------------------------------
void vsStreamFactory::setSource(vsStreamSource* s)
{
  QTE_D(vsStreamFactory);
  d->source = QSharedPointer<vsStreamSource>(s);
}

//-----------------------------------------------------------------------------
vsVideoSourcePtr vsStreamFactory::videoSource() const
{
  QTE_D_CONST(vsStreamFactory);
  return d->source;
}

//-----------------------------------------------------------------------------
QList<vsTrackSourcePtr> vsStreamFactory::trackSources() const
{
  QTE_D_CONST(vsStreamFactory);

  vsTrackSourcePtr s;
  if (!d->source || !(s = d->source->trackSource()))
    {
    return vsSourceFactory::trackSources();
    }

  QList<vsTrackSourcePtr> list;
  list.append(s);
  return list;
}

//-----------------------------------------------------------------------------
QList<vsDescriptorSourcePtr> vsStreamFactory::descriptorSources() const
{
  QTE_D_CONST(vsStreamFactory);

  vsDescriptorSourcePtr s;
  if (!d->source || !(s = d->source->descriptorSource()))
    {
    return vsSourceFactory::descriptorSources();
    }

  QList<vsDescriptorSourcePtr> list;
  list.append(s);
  return list;
}
