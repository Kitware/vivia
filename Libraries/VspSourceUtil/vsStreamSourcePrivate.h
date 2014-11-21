/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsStreamSourcePrivate_h
#define __vsStreamSourcePrivate_h

#include <QUrl>

#include <vgExport.h>

#include <vvDescriptor.h>

#include <vsDescriptor.h>
#include <vsDescriptorInput.h>
#include <vsEvent.h>
#include <vsEventInfo.h>
#include <vsVideoSourcePrivate.h>

#include "vsStreamSource.h"

class vsStreamTrackSource;
class vsStreamDescriptorSource;

class VSP_SOURCEUTIL_EXPORT vsStreamSourcePrivate : public vsVideoSourcePrivate
{
  Q_OBJECT

protected:
  QTE_DECLARE_PUBLIC(vsStreamSource)
  friend class vsStreamTrackSource;
  friend class vsStreamDescriptorSource;

  vsStreamSourcePrivate(vsStreamSource* q, QUrl streamUri);
  virtual ~vsStreamSourcePrivate();

  virtual QString text() const;
  virtual QString text(QString format) const = 0;
  virtual QString toolTip(const QString& sourceTypeSingular,
                          const QString& sourceTypePlural) const;

  void beginAcceptingInput();
  void updateStatus(vsDataSource::Status newStatus);
  void suicide();

  void notifyEventGroupExpected(vsEventInfo::Group group);
  void notifyClassifiersAvailable(bool);

  virtual vsDescriptorInput::Types inputAccepted() const;

  virtual void injectInput(qint64 id, vsDescriptorInputPtr input);
  virtual void revokeInput(qint64 id, bool revokeEvents);
  virtual void revokeAllInput(bool revokeEvents);

  const QUrl StreamUri;

signals:
  void descriptorAvailable(vsDescriptor);
  void descriptorsAvailable(vsDescriptorList);
  void eventAvailable(vsEvent);
  void eventRevoked(vtkIdType);

private:
  QTE_DISABLE_COPY(vsStreamSourcePrivate)

  QSharedPointer<vsStreamTrackSource> TrackSource;
  QSharedPointer<vsStreamDescriptorSource> DescriptorSource;
};

#endif
