// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
