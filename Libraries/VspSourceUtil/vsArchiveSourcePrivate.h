/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
