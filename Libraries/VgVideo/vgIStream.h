// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
