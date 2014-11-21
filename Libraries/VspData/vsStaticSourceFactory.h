/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsStaticSourceFactory_h
#define __vsStaticSourceFactory_h

#include <vgExport.h>

#include "vsSimpleSourceFactory.h"

class vsStaticSourceFactoryPrivate;

/// Static source factory.
///
/// This class implements a static source factory, which is a sort of "fake"
/// factory that must be pre-initialized with its sources. This is intended to
/// be used as a "source container" convenience class for archive source
/// plugins, such that their implementations of
/// ::vsArchiveSourceInterface::createArchiveSource can create raw sources
/// without a factory, stuff them into an instance of this class, and use said
/// instance as their return object.
class VSP_DATA_EXPORT vsStaticSourceFactory : public vsSimpleSourceFactory
{
public:
  vsStaticSourceFactory();
  virtual ~vsStaticSourceFactory();

  virtual vsVideoSourcePtr videoSource() const QTE_OVERRIDE;
  virtual QList<vsTrackSourcePtr> trackSources() const QTE_OVERRIDE;
  virtual QList<vsDescriptorSourcePtr> descriptorSources() const QTE_OVERRIDE;

  /// Set video source.
  ///
  /// This method sets the static factory's video source to the specified
  /// source. Only one video source is supported; if a video source was already
  /// set, it is replaces with the new source.
  void setVideoSource(const vsVideoSourcePtr&);

  /// \copydoc setVideoSource(vsVideoSourcePtr)
  ///
  /// This convenience overload creates a new ::QSharedPointer from a bare
  /// pointer. The object must not already be wrapped in a pointer class.
  void setVideoSource(vsVideoSource*);

  /// Add track source.
  ///
  /// This method adds a track source to the static factory. Multiple sources
  /// may be added.
  void addTrackSource(const vsTrackSourcePtr&);

  /// \copydoc addTrackSource(vsTrackSourcePtr)
  ///
  /// This convenience overload creates a new ::QSharedPointer from a bare
  /// pointer. The object must not already be wrapped in a pointer class.
  void addTrackSource(vsTrackSource*);

  /// Add descriptor source.
  ///
  /// This method adds a descriptor source to the static factory. Multiple
  /// sources may be added.
  void addDescriptorSource(const vsDescriptorSourcePtr&);

  /// \copydoc addDescriptorSource(vsDescriptorSourcePtr)
  ///
  /// This convenience overload creates a new ::QSharedPointer from a bare
  /// pointer. The object must not already be wrapped in a pointer class.
  void addDescriptorSource(vsDescriptorSource*);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsStaticSourceFactory)

private:
  QTE_DECLARE_PRIVATE(vsStaticSourceFactory)
  QTE_DISABLE_COPY(vsStaticSourceFactory)
};

#endif
