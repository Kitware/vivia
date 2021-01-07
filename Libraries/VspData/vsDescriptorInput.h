// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsDescriptorInput_h
#define __vsDescriptorInput_h

#include <QScopedPointer>
#include <QSharedPointer>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvIqr.h>

class QPolygonF;

struct vvSimilarityQuery;

class vtkImageData;

struct vgTimeStamp;

class vtkVgEventBase;
struct vtkVgVideoFrameMetaData;

class vsContour;
class vsEvent;
struct vsEventId;
struct vsTrackId;
struct vsTrackObjectClassifier;
struct vsTrackState;

class vsDescriptorInputPrivate;

class VSP_DATA_EXPORT vsDescriptorInput
{
public:
  enum Type
    {
    NoType = 0,
    TrackUpdate = 0x1,
    TrackClosure = 0x2,
    TrackObjectClassifier = 0x8,
    Event = 0x10,
    FrameMetaData = 0x40,
    EventRating = 0x100,
    EventNote = 0x200,
    TrackNote = 0x400,
    Contour = 0x1000,
    Query = 0x2000
    };
  Q_DECLARE_FLAGS(Types, Type)

  ~vsDescriptorInput();

  Type type() const;

  // constructors
  explicit vsDescriptorInput(const vtkVgVideoFrameMetaData& metaData);
  explicit vsDescriptorInput(const vsTrackId& closedTrack);
  explicit vsDescriptorInput(const vsTrackId&,
                             const vsTrackState&);
  explicit vsDescriptorInput(const vsTrackId&,
                             const vsTrackObjectClassifier&);

  explicit vsDescriptorInput(const vsEvent&, const vsEventId&);

  explicit vsDescriptorInput(const vsEventId&, const vvIqr::Classification&);
  explicit vsDescriptorInput(const vsEventId&, const QString&,
                             const vgTimeStamp&, const vgTimeStamp&);

  explicit vsDescriptorInput(const vsTrackId&, const QString&,
                             const vgTimeStamp&, const vgTimeStamp&);

  explicit vsDescriptorInput(const vsContour&);
  explicit vsDescriptorInput(const vvSimilarityQuery&, int id);

  // accessors
  const vtkVgVideoFrameMetaData*    frameMetaData() const;
  const vsTrackId*                  trackId() const;
  const vsTrackState*               trackState() const;
  const vsTrackObjectClassifier*    trackClassifier() const;

  const vsContour*                  contour() const;

  const vtkVgEventBase*             event() const;
  const vsEventId*                  eventId() const;

  const vvIqr::Classification*      rating() const;
  const QString*                    note() const;
  const vgTimeStamp*                noteStartTime() const;
  const vgTimeStamp*                noteEndTime() const;

  const vvSimilarityQuery*          query() const;
  const int*                        queryId() const;

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsDescriptorInput)

private:
  QTE_DECLARE_PRIVATE(vsDescriptorInput)
  QTE_DISABLE_COPY(vsDescriptorInput)
};

typedef QSharedPointer<const vsDescriptorInput> vsDescriptorInputPtr;

Q_DECLARE_OPERATORS_FOR_FLAGS(vsDescriptorInput::Types)

#endif
