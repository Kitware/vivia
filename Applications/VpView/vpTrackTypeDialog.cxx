/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpTrackTypeDialog.h"
#include "ui_vpTrackTypeDialog.h"

#include "vpTreeView.h"

#include <vgTrackType.h>

#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackTypeRegistry.h>

#include <QPushButton>

QTE_IMPLEMENT_D_FUNC(vpTrackTypeDialog)

namespace
{

}

//-----------------------------------------------------------------------------
class vpTrackTypeDialogPrivate
{
public:
  Ui::vpTrackTypeDialog UI;
  vpTreeView* TreeView;
  vtkVgTrackModel* TrackModel;
  vtkVgTrackTypeRegistry* TrackTypes;
};

//-----------------------------------------------------------------------------
vpTrackTypeDialog::vpTrackTypeDialog(vpTreeView* treeView,
                                     vtkVgTrackModel* trackModel,
                                     vtkVgTrackTypeRegistry* trackTypes,
                                     QWidget* parent)
  : QDialog(parent), d_ptr(new vpTrackTypeDialogPrivate)
{
  QTE_D(vpTrackTypeDialog);

  d->UI.setupUi(this);
  d->TreeView = treeView;
  d->TrackModel = trackModel;
  d->TrackTypes = trackTypes;

  this->BuildTypeList();
  connect(d->UI.trackType, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateTypeIndex(int)));
          
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
}

//-----------------------------------------------------------------------------
vpTrackTypeDialog::~vpTrackTypeDialog()
{
}

//-----------------------------------------------------------------------------
void vpTrackTypeDialog::BuildTypeList()
{
  QTE_D_CONST(vpTrackTypeDialog);

  d->UI.trackType->clear();
  d->UI.trackType->addItem("None");

  for (int i = 0, end = d->TrackTypes->GetNumberOfTypes(); i < end; ++i)
    {
    const char* name = d->TrackTypes->GetType(i).GetName();
    if (*name)
      {
      d->UI.trackType->addItem(name);
      }
    }
  d->UI.trackType->setEnabled(true);
  d->UI.trackType->setCurrentIndex(0);
}

//-----------------------------------------------------------------------------
void vpTrackTypeDialog::accept()
{
  QTE_D_CONST(vpTrackTypeDialog);

  QList<QTreeWidgetItem*> items = d->TreeView->selectedItems();

  foreach(QTreeWidgetItem* item, items)
    {
    int type, id, parentId, index;
    d->TreeView->GetItemInfo(item, type, id, parentId, index);
    d->TrackModel->GetTrack(id)->SetType(d->UI.trackType->currentIndex() - 1);
    }
  d->TrackModel->Modified();

  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vpTrackTypeDialog::updateTypeIndex(int index)
{
  QTE_D_CONST(vpTrackTypeDialog);

  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!!index);
}
