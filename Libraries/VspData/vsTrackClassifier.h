/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
