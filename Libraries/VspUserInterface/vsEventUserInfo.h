/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
