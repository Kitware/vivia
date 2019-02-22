/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsLineWidget_h
#define __vsLineWidget_h

#include <vgMatrix.h>

#include <qtGlobal.h>

#include <QObject>

class vtkMatrix4x4;
class vtkPoints;
class vtkRenderWindowInteractor;

class vsLineWidgetPrivate;

class vsLineWidget : public QObject
{
  Q_OBJECT

public:
  vsLineWidget(vtkRenderWindowInteractor*);
  ~vsLineWidget();

  void setMatrix(vtkMatrix4x4* matrix);
  double lineLength();

signals:
  void lineLengthChanged(double length);

public slots:
  void begin();
  void end();

  void setVisible(bool);
  void setMatrix(const vgMatrix4d& matrix);

protected slots:
  void updateLineLength();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsLineWidget)

private:
  QTE_DECLARE_PRIVATE(vsLineWidget)
  Q_DISABLE_COPY(vsLineWidget)
};

#endif
