// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
