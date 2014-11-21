/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
