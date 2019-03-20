/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
