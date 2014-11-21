/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpQtViewer3dDialog_h
#define __vpQtViewer3dDialog_h

#include <QDialog>

// Forward declarations
class vpQtViewer3dWidget;
class vtkVgTimeStamp;

class vpQtViewer3dDialog : public QDialog
{
  Q_OBJECT

public:
  explicit vpQtViewer3dDialog(QWidget* parent = 0);
  ~vpQtViewer3dDialog();

  vpQtViewer3dWidget* getViewer3dWidget();

  virtual void update(const vtkVgTimeStamp& timestamp);

  virtual void reset();


public slots:

  void onContextCreated();
  void onContextLODChanged(int value);


protected:
  virtual void setupInternalUi();

private:
  class vtkInternal;
  vtkInternal* Internal;
};

#endif // __vpQtViewer3dDialog_h
