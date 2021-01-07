// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventUserInfo_h
#define __vsEventUserInfo_h

#include <QString>

class vtkVgEvent;

class vsEventUserInfo
{
public:
  vtkVgEvent* Event;
  int Status;
};

#endif
