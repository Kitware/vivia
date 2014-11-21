/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgQtUtil_h
#define __vtkVgQtUtil_h

#include <QObject>

#include <vgExport.h>

class vtkObject;

extern VTKVGQT_UTIL_EXPORT void vtkConnect(
  vtkObject* sender, unsigned long event,
  QObject* receiver, const char* slot,
  Qt::ConnectionType type = Qt::AutoConnection);

extern VTKVGQT_UTIL_EXPORT void vtkConnect(
  vtkObject* sender, unsigned long event,
  QObject* receiver, const char* slot,
  void* data, float priority = 0.0f,
  Qt::ConnectionType type = Qt::AutoConnection);

extern VTKVGQT_UTIL_EXPORT void vtkDisconnect(
  vtkObject* sender = 0, unsigned long event = 0,
  QObject* receiver = 0, const char* slot = 0,
  void* data = 0);

#endif
