// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfTrackData_h
#define __vdfTrackData_h

template <class Key, class Value> class QHash;
template <class Value> class QSet;

class QString;

template <class Value> class vgTimeMap;

typedef QSet<QString> vdfTrackAttributes;
typedef vgTimeMap<double> vdfTrackScalarData;
typedef QHash<QString, double> vdfTrackStateScalars;
typedef QHash<QString, vdfTrackScalarData> vdfTrackScalarDataCollection;

#endif
