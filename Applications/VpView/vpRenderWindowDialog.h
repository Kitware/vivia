// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
