/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsLineWidget_h
#define __vsLineWidget_h

#include <QMatrix4x4>
#include <QObject>

#include <qtGlobal.h>

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
  void setMatrix(const QMatrix4x4& matrix);

protected slots:
  void updateLineLength();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsLineWidget)

private:
  QTE_DECLARE_PRIVATE(vsLineWidget)
  Q_DISABLE_COPY(vsLineWidget)
};

#endif
