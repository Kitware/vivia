/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgIStream_h
#define __vgIStream_h

#include <QScopedPointer>
#include <QString>

#include <qtGlobal.h>

#include <iosfwd>

#include <vgExport.h>

class vgIStreamPrivate;

class VG_VIDEO_EXPORT vgIStream
{
public:
  vgIStream();
  ~vgIStream();

  bool open(const QString& fileName);
  bool seek(qint64 position);

  operator std::istream*();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgIStream)

private:
  QTE_DECLARE_PRIVATE(vgIStream)
  Q_DISABLE_COPY(vgIStream)
};

#endif
