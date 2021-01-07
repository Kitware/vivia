// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsSimpleSourceFactory_h
#define __vsSimpleSourceFactory_h

#include <QSharedPointer>

#include <qtGlobal.h>

#include <vgExport.h>

template <typename T> class QList;

class vsVideoSource;
class vsTrackSource;
class vsDescriptorSource;

typedef QSharedPointer<vsVideoSource> vsVideoSourcePtr;
typedef QSharedPointer<vsTrackSource> vsTrackSourcePtr;
typedef QSharedPointer<vsDescriptorSource> vsDescriptorSourcePtr;

/// Simple source factory.
///
/// This class implements a simple source factory that does not support
/// initialization. Source factory plugins must use ::vsSourceFactory, which
/// does support initialization. Archive source plugins may wish to use
/// ::vsStaticSourceFactory, which serves as a data source container which
/// can be given sources at creation, prior to being returned from
/// ::vsArchiveSourceInterface::createArchiveSource.
class VSP_DATA_EXPORT vsSimpleSourceFactory
{
public:
  virtual ~vsSimpleSourceFactory();

  /// Return the video source associated with this factory.
  ///
  /// This method returns a shared pointer to the video source provided by this
  /// factory. The pointer will be null if the factory does not provide a video
  /// source.
  virtual vsVideoSourcePtr videoSource() const;

  /// Return the track sources associated with this factory.
  ///
  /// This method returns a list of shared pointers to the track source(s)
  /// provided by this factory. The list will be empty if the factory does not
  /// provide track sources.
  virtual QList<vsTrackSourcePtr> trackSources() const;

  /// Return the descriptor sources associated with this factory.
  ///
  /// This method returns a list of shared pointers to the descriptor source(s)
  /// provided by this factory. The list will be empty if the factory does not
  /// provide descriptor sources.
  virtual QList<vsDescriptorSourcePtr> descriptorSources() const;

protected:
  vsSimpleSourceFactory();

private:
  QTE_DISABLE_COPY(vsSimpleSourceFactory)
};

typedef QSharedPointer<vsSimpleSourceFactory> vsSimpleSourceFactoryPtr;

#endif
