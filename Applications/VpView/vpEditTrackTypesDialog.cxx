/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpEditTrackTypesDialog.h"

#include "ui_vpEditTrackTypesDialog.h"

#include <QInputDialog>
#include <QMessageBox>

#include "vpTrackConfig.h"

#include <vgTrackType.h>

#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>

//-----------------------------------------------------------------------------
class vpEditTrackTypesDialogPrivate
{
public:
  Ui_vpEditTrackTypesDialog UI;

  vpTrackConfig* TrackTypes;
  vtkVgTrackModel* TrackModel;
};

QTE_IMPLEMENT_D_FUNC(vpEditTrackTypesDialog)

//-----------------------------------------------------------------------------
vpEditTrackTypesDialog::vpEditTrackTypesDialog(vpTrackConfig* trackTypes,
                                               vtkVgTrackModel* trackModel,
                                               QWidget* parent,
                                               Qt::WindowFlags flags) :
  QDialog(parent, flags), d_ptr(new vpEditTrackTypesDialogPrivate)
{
  QTE_D(vpEditTrackTypesDialog);

  d->UI.setupUi(this);

  d->TrackTypes = trackTypes;
  d->TrackModel = trackModel;

  connect(d->UI.remove, SIGNAL(clicked()), this, SLOT(remove()));

  this->buildTypesList();
}

//-----------------------------------------------------------------------------
vpEditTrackTypesDialog::~vpEditTrackTypesDialog()
{
}

//-----------------------------------------------------------------------------
void vpEditTrackTypesDialog::remove()
{
  QTE_D(vpEditTrackTypesDialog);

  QListWidgetItem* item = d->UI.types->currentItem();
  if (!item)
    {
    return;
    }

  int idx = d->TrackTypes->GetTrackTypeIndex(qPrintable(item->text()));
  if (idx == -1)
    {
    return;
    }

  vgTrackType type = d->TrackTypes->GetTrackTypeByIndex(idx);

  // Make sure no tracks are using the type to be removed
  vtkVgTrackInfo trackInfo;
  d->TrackModel->InitTrackTraversal();
  while ((trackInfo = d->TrackModel->GetNextTrack()).GetTrack())
    {
    if (trackInfo.GetTrack()->GetType() == idx)
      {
      QMessageBox::warning(this, QString(),
                           QString("One or more tracks have the type \"%1\".")
                           .arg(type.GetId()));
      return;
      }
    }

  if (QMessageBox::warning(this, QString(),
                           QString("Are you sure you want to remove \"%1\"?")
                           .arg(type.GetId()),
                           QMessageBox::Ok | QMessageBox::Cancel) !=
      QMessageBox::Ok)
    {
    return;
    }

  // For now, indicate the type is invalid by setting its name to the empty
  // string. We can't remove the type altogether since tracks currently refer to
  // types by index.
  type.SetId("");
  d->TrackTypes->SetType(idx, type);

  this->buildTypesList();
}

//-----------------------------------------------------------------------------
void vpEditTrackTypesDialog::buildTypesList()
{
  QTE_D(vpEditTrackTypesDialog);

  d->UI.types->clear();
  for (int i = 0, n = d->TrackTypes->GetNumberOfTypes(); i < n; ++i)
    {
    const vgTrackType& type = d->TrackTypes->GetTrackTypeByIndex(i);
    const char *name = type.GetName();
    if (*name)
      {
      d->UI.types->addItem(new QListWidgetItem(name));
      }
    }
}
