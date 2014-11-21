/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsKwaVideoArchiveSource_h
#define __vsKwaVideoArchiveSource_h

#include <QUrl>

#include <vsVideoSource.h>

class vsKwaVideoArchiveSourcePrivate;

class vsKwaVideoArchiveSource : public vsVideoSource
{
  Q_OBJECT

public:
  vsKwaVideoArchiveSource(QUrl archiveUri);
  virtual ~vsKwaVideoArchiveSource();

  virtual QString text() const QTE_OVERRIDE;
  virtual QString toolTip() const QTE_OVERRIDE;

protected slots:
  virtual void requestFrame(vgVideoSeekRequest) QTE_OVERRIDE;

protected:
  virtual void timerEvent(QTimerEvent*) QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vsKwaVideoArchiveSource)
  QTE_DISABLE_COPY(vsKwaVideoArchiveSource)
};

#endif
