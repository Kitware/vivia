/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfTrackReaderPrivate_h
#define __vdfTrackReaderPrivate_h

#include "vdfTrackReader.h"

#include <QHash>

class vdfTrackReaderPrivate : public QObject
{
  Q_OBJECT

public:
  vdfTrackReaderPrivate() {}
  ~vdfTrackReaderPrivate() {}

  QHash<vdfTrackId, vdfTrackReader::Track> Tracks;

public slots:
  void setTrackName(const vdfTrackId& trackId, const QString& name);

  void setTrackClassification(const vdfTrackId& trackId,
                              const vvTrackObjectClassification& toc);

  void setTrackState(const vdfTrackId& trackId, vvTrackState state);
  void setTrackStates(const vdfTrackId& trackId,
                      const QList<vvTrackState>& states);

private:
  QTE_DISABLE_COPY(vdfTrackReaderPrivate)
};

#endif
