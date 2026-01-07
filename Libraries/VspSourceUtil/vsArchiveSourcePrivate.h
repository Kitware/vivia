// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsArchiveSourcePrivate_h
#define __vsArchiveSourcePrivate_h

#include <qtThread.h>

#include <vgExport.h>

#include "vsArchiveSource.h"

//-----------------------------------------------------------------------------
class VSP_SOURCEUTIL_EXPORT vsArchiveSourcePrivate : public qtThread
{
public:
  vsArchiveSourcePrivate(
    vsArchiveSourceHelper* q, const QString& type, const QUrl& uri) :
    q_ptr(q), Active(true), Type(type), ArchiveUri(uri) {}

  virtual ~vsArchiveSourcePrivate();

protected:
  QTE_DECLARE_PUBLIC_PTR(vsArchiveSourceHelper)

  virtual void run() QTE_OVERRIDE;
  virtual bool isRunning() const QTE_OVERRIDE;

  virtual bool processArchive(const QUrl& uri) = 0;

private:
  QTE_DECLARE_PUBLIC(vsArchiveSourceHelper)
  QTE_DISABLE_COPY(vsArchiveSourcePrivate)

  volatile bool Active;
  QString Type;
  QUrl ArchiveUri;
};

#endif
