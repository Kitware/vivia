// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
