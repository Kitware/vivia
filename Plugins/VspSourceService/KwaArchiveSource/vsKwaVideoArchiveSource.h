// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
