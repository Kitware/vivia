/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsDescriptorSource_h
#define __vsDescriptorSource_h

#include <vgExport.h>

#include <vvDescriptor.h>
#include <vvTrack.h>

#include "vsDataSource.h"
#include "vsDescriptor.h"
#include "vsEvent.h"
#include "vsEventInfo.h"
#include "vsTrackClassifier.h"

#include "vsDescriptorInput.h"

class VSP_DATA_EXPORT vsDescriptorSource : public vsDataSource
{
  Q_OBJECT

public:
  virtual ~vsDescriptorSource() {}

  virtual vsDescriptorInput::Types inputAccepted() const
    { return vsDescriptorInput::NoType; }

signals:
  void readyForInput(vsDescriptorSource* source);

  void eventTypeAvailable(vsDescriptorSource* source, vsEventInfo info,
                          double initialFilterThreshold);
  void eventGroupExpected(vsEventInfo::Group);

  void descriptorAvailable(vsDescriptorSource* source, vsDescriptor ds);
  void descriptorsAvailable(vsDescriptorSource* source, vsDescriptorList dl);
  void eventAvailable(vsDescriptorSource* source, vsEvent event);
  void eventRevoked(vsDescriptorSource* source, vtkIdType eventId);

  void tocAvailable(vvTrackId trackId, vsTrackObjectClassifier toc);

  void destroyed(vsDescriptorSource*);

public slots:
  virtual void injectInput(qint64 id, vsDescriptorInputPtr input)
    { Q_UNUSED(id); Q_UNUSED(input); }

  virtual void revokeInput(qint64 id, bool revokeEvents)
    { Q_UNUSED(id); Q_UNUSED(revokeEvents); }

  virtual void revokeAllInput(bool revokeEvents)
    { Q_UNUSED(revokeEvents); }

protected slots:
  void emitEventType(vsEventInfo info, double initialFilterThreshold)
    { emit this->eventTypeAvailable(this, info, initialFilterThreshold); }
  void emitDescriptor(vsDescriptor ds)
    { emit this->descriptorAvailable(this, ds); }
  void emitDescriptors(vsDescriptorList dl)
    { emit this->descriptorsAvailable(this, dl); }
  void emitEvent(vsEvent event)
    { emit this->eventAvailable(this, event); }
  void revokeEvent(vtkIdType id)
    { emit this->eventRevoked(this, id); }

protected:
  vsDescriptorSource() {}

  void suicide() { emit this->destroyed(this); }

private:
  QTE_DISABLE_COPY(vsDescriptorSource)
};

#endif
