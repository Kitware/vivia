/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvAbstractQueryServerChooser_h
#define __vvAbstractQueryServerChooser_h

#include <QUrl>
#include <QWidget>

#include <vgExport.h>

class VV_WIDGETS_EXPORT vvAbstractQueryServerChooser : public QWidget
{
  Q_OBJECT

public:
  vvAbstractQueryServerChooser(QWidget* parent = 0) : QWidget(parent) {}
  virtual ~vvAbstractQueryServerChooser() {}

  virtual QUrl uri() const = 0;

signals:
  void uriChanged(QUrl);

public slots:
  virtual void setUri(QUrl) = 0;

private:
  Q_DISABLE_COPY(vvAbstractQueryServerChooser)
};

#endif
