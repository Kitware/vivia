/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "moc_vsStreamSourcePrivate.cpp"

#include <vgCheckArg.h>

#include <vsTrackSource.h>
#include <vsDescriptorSource.h>
#include <vtkVsTrackInfo.h>

QTE_IMPLEMENT_D_FUNC(vsStreamSource)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsStreamTrackSource

//-----------------------------------------------------------------------------
class vsStreamTrackSource : public vsTrackSource
{
public:
  virtual ~vsStreamTrackSource() {}

  virtual Status status() const;
  virtual QString text() const;
  virtual QString toolTip() const;

protected:
  QTE_DECLARE_PUBLIC_PTR(vsStreamSourcePrivate)

  vsStreamTrackSource(vsStreamSourcePrivate* q) : q_ptr(q) {}

private:
  QTE_DECLARE_PUBLIC(vsStreamSourcePrivate)
};

//-----------------------------------------------------------------------------
vsDataSource::Status vsStreamTrackSource::status() const
{
  QTE_Q_CONST(vsStreamSourcePrivate);
  return q->Status;
}

//-----------------------------------------------------------------------------
QString vsStreamTrackSource::text() const
{
  QTE_Q_CONST(vsStreamSourcePrivate);
  return q->text();
}

//-----------------------------------------------------------------------------
QString vsStreamTrackSource::toolTip() const
{
  QTE_Q_CONST(vsStreamSourcePrivate);
  return q->toolTip("track", "tracks");
}

//END vsStreamTrackSource

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsStreamDescriptorSource

//-----------------------------------------------------------------------------
class vsStreamDescriptorSource : public vsDescriptorSource
{
public:
  virtual ~vsStreamDescriptorSource() {}

  virtual Status status() const;
  virtual QString text() const;
  virtual QString toolTip() const;

  virtual vsDescriptorInput::Types inputAccepted() const;

  virtual void injectInput(qint64 id, vsDescriptorInputPtr input);
  virtual void revokeInput(qint64 id, bool revokeEvents);
  virtual void revokeAllInput(bool revokeEvents);

protected:
  QTE_DECLARE_PUBLIC_PTR(vsStreamSourcePrivate)

  vsStreamDescriptorSource(vsStreamSourcePrivate* q) : q_ptr(q) {}

private:
  QTE_DECLARE_PUBLIC(vsStreamSourcePrivate)
};

//-----------------------------------------------------------------------------
vsDataSource::Status vsStreamDescriptorSource::status() const
{
  QTE_Q_CONST(vsStreamSourcePrivate);
  return q->Status;
}

//-----------------------------------------------------------------------------
QString vsStreamDescriptorSource::text() const
{
  QTE_Q_CONST(vsStreamSourcePrivate);
  return q->text();
}

//-----------------------------------------------------------------------------
QString vsStreamDescriptorSource::toolTip() const
{
  QTE_Q_CONST(vsStreamSourcePrivate);
  return q->toolTip("descriptor", "descriptors");
}

//-----------------------------------------------------------------------------
vsDescriptorInput::Types vsStreamDescriptorSource::inputAccepted() const
{
  QTE_Q_CONST(vsStreamSourcePrivate);
  return q->inputAccepted() | vsDescriptorSource::inputAccepted();
}

//-----------------------------------------------------------------------------
void vsStreamDescriptorSource::injectInput(
  qint64 id, vsDescriptorInputPtr input)
{
  QTE_Q(vsStreamSourcePrivate);
  q->injectInput(id, input);
  vsDescriptorSource::injectInput(id, input);
}

//-----------------------------------------------------------------------------
void vsStreamDescriptorSource::revokeInput(qint64 id, bool revokeEvents)
{
  QTE_Q(vsStreamSourcePrivate);
  q->revokeInput(id, revokeEvents);
  vsDescriptorSource::revokeInput(id, revokeEvents);
}

//-----------------------------------------------------------------------------
void vsStreamDescriptorSource::revokeAllInput(bool revokeEvents)
{
  QTE_Q(vsStreamSourcePrivate);
  q->revokeAllInput(revokeEvents);
  vsDescriptorSource::revokeAllInput(revokeEvents);
}

//END vsStreamDescriptorSource

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsStreamSourcePrivate

//-----------------------------------------------------------------------------
vsStreamSourcePrivate::vsStreamSourcePrivate(
  vsStreamSource* q, QUrl streamUri) :
  vsVideoSourcePrivate(q), StreamUri(streamUri),
  TrackSource(new vsStreamTrackSource(this)),
  DescriptorSource(new vsStreamDescriptorSource(this))
{
  this->TrackSource->moveToThread(this->thread());
  this->DescriptorSource->moveToThread(this->thread());
}

//-----------------------------------------------------------------------------
vsStreamSourcePrivate::~vsStreamSourcePrivate()
{
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::beginAcceptingInput()
{
  emit this->DescriptorSource->readyForInput(this->DescriptorSource.data());
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::updateStatus(vsDataSource::Status newStatus)
{
  // This method is meant to be called from our thread to post an update to the
  // GUI thread, so we use invokeMethod to pass the call across
  QTE_Q(vsStreamSource);
  QMetaObject::invokeMethod(q, "updateStatus", Qt::QueuedConnection,
                            Q_ARG(vsDataSource::Status, newStatus));
  bool streaming =
    (newStatus == vsDataSource::StreamingActive ||
     newStatus == vsDataSource::StreamingPending);
  QMetaObject::invokeMethod(q, "setStreaming", Qt::QueuedConnection,
                            Q_ARG(bool, streaming));
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::suicide()
{
  this->DescriptorSource->suicide();
  this->TrackSource->suicide();
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::notifyClassifiersAvailable(bool available)
{
  CHECK_ARG(available);
  this->notifyEventGroupExpected(vsEventInfo::Classifier);
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::notifyEventGroupExpected(vsEventInfo::Group group)
{
  emit this->DescriptorSource->eventGroupExpected(group);
}

//-----------------------------------------------------------------------------
QString vsStreamSourcePrivate::text() const
{
  // Get appropriate format
  QString format;
  switch (this->Status)
    {
    case vsDataSource::StreamingPending:
      format = "(P %1)";
      break;
    case vsDataSource::StreamingActive:
      format = "A %1";
      break;
    case vsDataSource::StreamingIdle:
      format = "(I %1)";
      break;
    case vsDataSource::StreamingStopped:
      format = "(C %1)";
      break;
    default:
      return "(none)";
    }

  // Format text
  return this->text(format);
}

//-----------------------------------------------------------------------------
QString vsStreamSourcePrivate::toolTip(
  const QString& sourceTypeSingular, const QString& sourceTypePlural) const
{
  // Get appropriate format
  QString format;
  const QString* sourceType;
  switch (this->Status)
    {
    case vsDataSource::StreamingPending:
      format = "Waiting for %1 stream from %2";
      sourceType = &sourceTypeSingular;
      break;
    case vsDataSource::StreamingActive:
      format = "Streaming %1 from %2";
      sourceType = &sourceTypePlural;
      break;
    case vsDataSource::StreamingIdle:
      format = "Idle %1 stream (from %2)";
      sourceType = &sourceTypeSingular;
      break;
    case vsDataSource::StreamingStopped:
      format = "Closed %1 stream (from %2)";
      sourceType = &sourceTypeSingular;
      break;
    default:
      return QString("(no %1 source)").arg(sourceTypeSingular);
    }

  // Return formatted text
  return format.arg(*sourceType).arg(this->StreamUri.toString());
}

//-----------------------------------------------------------------------------
vsDescriptorInput::Types vsStreamSourcePrivate::inputAccepted() const
{
  return vsDescriptorInput::NoType;
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::injectInput(
  qint64 id, vsDescriptorInputPtr input)
{
  Q_UNUSED(id);
  Q_UNUSED(input);
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::revokeInput(qint64 id, bool revokeEvents)
{
  Q_UNUSED(id);
  Q_UNUSED(revokeEvents);
}

//-----------------------------------------------------------------------------
void vsStreamSourcePrivate::revokeAllInput(bool revokeEvents)
{
  Q_UNUSED(revokeEvents);
}

//END vsStreamSourcePrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsStreamSource

//-----------------------------------------------------------------------------
vsStreamSource::vsStreamSource(vsStreamSourcePrivate* d) : vsVideoSource(d)
{
  connect(this, SIGNAL(statusChanged(vsDataSource::Status)),
          d->TrackSource.data(),
          SIGNAL(statusChanged(vsDataSource::Status)));
  connect(this, SIGNAL(statusChanged(vsDataSource::Status)),
          d->DescriptorSource.data(),
          SIGNAL(statusChanged(vsDataSource::Status)));
  this->updateStatus(vsDataSource::StreamingPending);
}

//-----------------------------------------------------------------------------
vsStreamSource::~vsStreamSource()
{
  QTE_D(vsStreamSource);
  d->suicide();
}

//-----------------------------------------------------------------------------
QString vsStreamSource::text() const
{
  QTE_D_CONST(vsStreamSource);
  return d->text();
}

//-----------------------------------------------------------------------------
QString vsStreamSource::toolTip() const
{
  QTE_D_CONST(vsStreamSource);
  return d->toolTip("video", "video");
}

//-----------------------------------------------------------------------------
vsTrackSourcePtr vsStreamSource::trackSource()
{
  QTE_D(vsStreamSource);
  return d->TrackSource;
}

//-----------------------------------------------------------------------------
vsDescriptorSourcePtr vsStreamSource::descriptorSource()
{
  QTE_D_CONST(vsStreamSource);
  return d->DescriptorSource;
}

//END vsStreamSource
