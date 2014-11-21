/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpRenderWindowDialog_h
#define __vpRenderWindowDialog_h

#include <QDialog>

class vtkRenderWindow;

namespace Ui
{
class qtRenderWindowDialog;
};

class vpRenderWindowDialog : public QDialog
{
  Q_OBJECT
public:

  vpRenderWindowDialog(QWidget* parent = NULL, Qt::WindowFlags flags = 0);
  virtual ~vpRenderWindowDialog();

  // Description:
  // Returns the vtkRenderWindow of the QVTK widget.
  vtkRenderWindow* GetRenderWindow();

signals:

protected:
  Ui::qtRenderWindowDialog* InternalWidget;

protected slots:
};


#endif /* __vpRenderWindowDialog_h */
