// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsTrackClassifier_h
#define __vsTrackClassifier_h

struct vsTrackObjectClassifier
{
  vsTrackObjectClassifier()
    : probabilityPerson(0.0), probabilityVehicle(0.0), probabilityOther(0.0) {}

  double probabilityPerson;
  double probabilityVehicle;
  double probabilityOther;
};

#endif
