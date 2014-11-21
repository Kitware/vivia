/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpInformaticsDialog_h
#define __vpInformaticsDialog_h

#include <QDialog>

#include <vtkSmartPointer.h>   // Required for smart pointer internal ivars.

class QVTKWidget;
class vtkRenderWindow;
class vtkRenderer;
class vtkGraph;
class vtkGraphLayoutView;

namespace Ui
{
class qtInformaticsDialog;
};

class vpInformaticsDialog : public QDialog
{
  Q_OBJECT
public:

  vpInformaticsDialog(QWidget* parent = NULL, Qt::WindowFlags flags = 0);
  virtual ~vpInformaticsDialog();

  // Description:
  // Returns the vtkRenderWindow of the QVTK widget.
  vtkRenderWindow* GetRenderWindow();

  // Description:
  // Set the vtkRenderWindow to the QVTK widget.
  void SetRenderWindow(vtkRenderWindow* renWin);

  // Description:
  // Get the QVTKWidget.
  QVTKWidget* GetWidget();

  void Initialize();

  void SetIconsFile(const std::string& filename)
    { IconsFile = filename; }

protected slots:

  virtual void onLayoutChanged(int index);
  virtual void onIconSizeChanged(int size);

signals:

private:
  void updateLayout();
  void updateIconSize();

  Ui::qtInformaticsDialog* InternalWidget;

  vtkSmartPointer<vtkRenderer> Renderer;
  vtkSmartPointer<vtkGraph> Graph;
  vtkSmartPointer<vtkGraphLayoutView> GraphView;

  std::string IconsFile;
};


#endif /* __vpInformaticsDialog_h */
