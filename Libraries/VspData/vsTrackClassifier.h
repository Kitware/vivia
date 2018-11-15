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
    : probabilityFish(0.0), probabilityScallop(0.0), probabilityOther(0.0) {}

  double probabilityFish;
  double probabilityScallop;
  double probabilityOther;
};

#endif
