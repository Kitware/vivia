// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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

  void setTrackState(const vdfTrackId& trackId, const vvTrackState& state,
                     const vdfTrackAttributes& attributes,
                     const vdfTrackStateScalars& scalarData);
  void setTrackStates(const vdfTrackId& trackId,
                      const QList<vvTrackState>& states,
                      const vgTimeMap<vdfTrackAttributes>& attributes,
                      const vdfTrackScalarDataCollection& scalarData);

private:
  QTE_DISABLE_COPY(vdfTrackReaderPrivate)
};

#endif
