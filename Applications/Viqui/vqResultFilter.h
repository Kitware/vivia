// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqResultFilter_h
#define __vqResultFilter_h

struct vqResultFilter
{
  vqResultFilter();

  bool isNoop();

  double Threshold;
};

#endif
