/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvDescriptorInfoWidget_h
#define __vvDescriptorInfoWidget_h

#include <QTabWidget>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvDescriptor.h>

class vvDescriptorInfoWidgetPrivate;

class VV_WIDGETS_EXPORT vvDescriptorInfoWidget : public QTabWidget
{
  Q_OBJECT

public:
  explicit vvDescriptorInfoWidget(QWidget* parent = 0);
  virtual ~vvDescriptorInfoWidget();

public slots:
  void setDescriptor(vvDescriptor);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvDescriptorInfoWidget)

private:
  QTE_DECLARE_PRIVATE(vvDescriptorInfoWidget)
  Q_DISABLE_COPY(vvDescriptorInfoWidget)
};

#endif
