/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpTrackTypeDialog_h
#define __vpTrackTypeDialog_h

#include <QDialog>

#include <qtGlobal.h>

class vgAttributeSet;
class vpTreeView;
class vtkVgTrackModel;
class vtkVgTrackTypeRegistry;

class vpTrackTypeDialogPrivate;

class vpTrackTypeDialog : public QDialog
{
  Q_OBJECT

public:
  vpTrackTypeDialog(vpTreeView* treeView, vtkVgTrackModel* trackModel,
                    vtkVgTrackTypeRegistry* trackTypes, QWidget* parent = 0);
  ~vpTrackTypeDialog();

signals:
  void updateRequested(vpTrackTypeDialog* dlg);

public slots:
  virtual void accept();

protected slots:
  void updateTypeIndex(int index);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpTrackTypeDialog)

private:
  QTE_DECLARE_PRIVATE(vpTrackTypeDialog)
  Q_DISABLE_COPY(vpTrackTypeDialog)
  
  void BuildTypeList();
};

#endif
