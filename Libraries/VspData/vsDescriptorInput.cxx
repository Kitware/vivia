/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsDescriptorInput.h"

#include <QPolygonF>
#include <QScopedPointer>

#include <vvQuery.h>
#include <vvTrack.h>

#include <vtkImageData.h>

#include <vgTimeStamp.h>

#include <vtkVgVideoFrameMetaData.h>

#include "vsContour.h"
#include "vsEvent.h"
#include "vsTrackClassifier.h"
#include "vsTrackId.h"
#include "vsTrackState.h"

QTE_IMPLEMENT_D_FUNC(vsDescriptorInput)

//-----------------------------------------------------------------------------
class vsDescriptorInputPrivate
{
public:
  vsDescriptorInput::Type Type;

  QScopedPointer<const vtkVgVideoFrameMetaData> FrameMetaData;

  QScopedPointer<const vsTrackId> TrackId;
  QScopedPointer<const vsTrackState> TrackState;
  QScopedPointer<const vsTrackObjectClassifier> TrackClassifier;

  QScopedPointer<const vsEvent> Event;
  QScopedPointer<const vsEventId> EventId;

  QScopedPointer<const vvIqr::Classification> Rating;
  QScopedPointer<const QString> Note;
  QScopedPointer<const vgTimeStamp> NoteStartTime;
  QScopedPointer<const vgTimeStamp> NoteEndTime;

  QScopedPointer<const vsContour> Contour;

  QScopedPointer<vvSimilarityQuery> Query;
  QScopedPointer<const int> QueryId;
};

//-----------------------------------------------------------------------------
vsDescriptorInput::~vsDescriptorInput()
{
}

//-----------------------------------------------------------------------------
vsDescriptorInput::Type vsDescriptorInput::type() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->Type;
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vtkVgVideoFrameMetaData& metaData) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::FrameMetaData;
  d->FrameMetaData.reset(new vtkVgVideoFrameMetaData(metaData));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsTrackId& closedTrack) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::TrackClosure;
  d->TrackId.reset(new vsTrackId(closedTrack));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsTrackId& id,
                                     const vsTrackState& data) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::TrackUpdate;
  d->TrackId.reset(new vsTrackId(id));
  d->TrackState.reset(new vsTrackState(data));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsTrackId& id,
                                     const vsTrackObjectClassifier& data) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::TrackObjectClassifier;
  d->TrackId.reset(new vsTrackId(id));
  d->TrackClassifier.reset(new vsTrackObjectClassifier(data));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsEvent& data, const vsEventId& id) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::Event;
  d->Event.reset(new vsEvent(data));
  d->EventId.reset(new vsEventId(id));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsEventId& id,
                                     const vvIqr::Classification& data) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::EventRating;
  d->EventId.reset(new vsEventId(id));
  d->Rating.reset(new vvIqr::Classification(data));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsEventId& id,
                                     const QString& note,
                                     const vgTimeStamp& startTime,
                                     const vgTimeStamp& endTime) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::EventNote;
  d->EventId.reset(new vsEventId(id));
  d->Note.reset(new QString(note));
  d->NoteStartTime.reset(new vgTimeStamp(startTime));
  d->NoteEndTime.reset(new vgTimeStamp(endTime));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsTrackId& id,
                                     const QString& note,
                                     const vgTimeStamp& startTime,
                                     const vgTimeStamp& endTime) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::TrackNote;
  d->TrackId.reset(new vsTrackId(id));
  d->Note.reset(new QString(note));
  d->NoteStartTime.reset(new vgTimeStamp(startTime));
  d->NoteEndTime.reset(new vgTimeStamp(endTime));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vsContour& contour) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::Contour;
  d->Contour.reset(new vsContour(contour));
}

//-----------------------------------------------------------------------------
vsDescriptorInput::vsDescriptorInput(const vvSimilarityQuery& data, int id) :
  d_ptr(new vsDescriptorInputPrivate)
{
  QTE_D(vsDescriptorInput);
  d->Type = vsDescriptorInput::Query;
  d->Query.reset(new vvSimilarityQuery(data));
  d->QueryId.reset(new int(id));
}

//-----------------------------------------------------------------------------
const vtkVgVideoFrameMetaData* vsDescriptorInput::frameMetaData() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->FrameMetaData.data();
}

//-----------------------------------------------------------------------------
const vsTrackId* vsDescriptorInput::trackId() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->TrackId.data();
}

//-----------------------------------------------------------------------------
const vsTrackState* vsDescriptorInput::trackState() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->TrackState.data();
}

//-----------------------------------------------------------------------------
const vsTrackObjectClassifier* vsDescriptorInput::trackClassifier() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->TrackClassifier.data();
}

//-----------------------------------------------------------------------------
const vsContour* vsDescriptorInput::contour() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->Contour.data();
}

//-----------------------------------------------------------------------------
const vtkVgEventBase* vsDescriptorInput::event() const
{
  QTE_D_CONST(vsDescriptorInput);
  return (d->Event ? d->Event.data()->GetVolatileConstPointer() : 0);
}

//-----------------------------------------------------------------------------
const vsEventId* vsDescriptorInput::eventId() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->EventId.data();
}

//-----------------------------------------------------------------------------
const vvIqr::Classification* vsDescriptorInput::rating() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->Rating.data();
}

//-----------------------------------------------------------------------------
const QString* vsDescriptorInput::note() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->Note.data();
}

//-----------------------------------------------------------------------------
const vgTimeStamp* vsDescriptorInput::noteStartTime() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->NoteStartTime.data();
}

//-----------------------------------------------------------------------------
const vgTimeStamp* vsDescriptorInput::noteEndTime() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->NoteEndTime.data();
}

//-----------------------------------------------------------------------------
const vvSimilarityQuery* vsDescriptorInput::query() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->Query.data();
}

//-----------------------------------------------------------------------------
const int* vsDescriptorInput::queryId() const
{
  QTE_D_CONST(vsDescriptorInput);
  return d->QueryId.data();
}
