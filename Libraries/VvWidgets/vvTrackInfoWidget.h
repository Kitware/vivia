/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
