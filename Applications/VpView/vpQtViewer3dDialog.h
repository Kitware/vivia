// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
