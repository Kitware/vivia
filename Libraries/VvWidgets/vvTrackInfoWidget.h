// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvTrackInfoWidget_h
#define __vvTrackInfoWidget_h

#include <QTabWidget>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvTrack.h>

class vvTrackInfoWidgetPrivate;

class VV_WIDGETS_EXPORT vvTrackInfoWidget : public QTabWidget
{
  Q_OBJECT

public:
  explicit vvTrackInfoWidget(QWidget* parent = 0);
  virtual ~vvTrackInfoWidget();

public slots:
  void setTrack(vvTrack);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvTrackInfoWidget)

private:
  QTE_DECLARE_PRIVATE(vvTrackInfoWidget)
  Q_DISABLE_COPY(vvTrackInfoWidget)
};

#endif
