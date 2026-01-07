// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpEditTrackTypesDialog_h
#define __vpEditTrackTypesDialog_h

#include <QDialog>

#include <qtGlobal.h>

class vpEditTrackTypesDialogPrivate;
class vpTrackConfig;
class vtkVgTrackModel;

class vpEditTrackTypesDialog : public QDialog
{
  Q_OBJECT

public:
  vpEditTrackTypesDialog(vpTrackConfig* trackTypes,
                         vtkVgTrackModel* trackModel,
                         QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vpEditTrackTypesDialog();

private slots:
  void remove();

private:
  void buildTypesList();

private:
  QTE_DECLARE_PRIVATE_RPTR(vpEditTrackTypesDialog)
  QTE_DECLARE_PRIVATE(vpEditTrackTypesDialog)
};

#endif // __vpEditTrackTypesDialog_h
