// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "ui_vpTrackAttributesPanel.h"

#include "vpTrackAttributesPanel.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegExpValidator>

#include <qtStlUtil.h>
#include <qtUtil.h>

#include "vgAttributeSet.h"
#include "vgUnixTime.h"

#include "vpProject.h"
#include "vpUtils.h"
#include "vpViewCore.h"
#include "vtkVpTrackModel.h"

#include "vtkVgScalars.h"

#include "vtkVgTrack.h"

//-----------------------------------------------------------------------------
vpTrackAttributesPanel::vpTrackAttributesPanel(QWidget* p)
  : QWidget(p), Track(0), Project(0), Editing(false)

{
  this->Ui = new Ui::vpTrackAttributesPanel;
  this->Ui->setupUi(this);

  connect(this->Ui->displayActiveAttributes, SIGNAL(stateChanged(int)),
    this, SLOT(ToggleDisplayOnlyActiveAttributes(int)));

  connect(this->Ui->attributeTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(UpdateTrackAttribute(QTreeWidgetItem*)));

  this->Ui->attributeTree->setEnabled(false);
  this->ShowTrackAttributesControls(false);
}

//-----------------------------------------------------------------------------
vpTrackAttributesPanel::~vpTrackAttributesPanel()
{
  delete this->Ui;
}

//-----------------------------------------------------------------------------
void vpTrackAttributesPanel::Initialize(vpViewCore* viewCore,
                                        vpProject* project)
{
  if (project == this->Project)
    {
    return;
    }

  this->ViewCoreInstance = viewCore;
  this->Project = project;

  this->Editing = false;

  this->BuildAttributeTree();
}

//-----------------------------------------------------------------------------
void vpTrackAttributesPanel::ShowTrackAttributesControls(bool state)
{
  if (state && this->Ui->attributeTree->topLevelItemCount() == 0)
    {
    state = false;
    }
  this->Ui->attributeTree->setVisible(state);
  this->Ui->displayActiveAttributes->setVisible(state);
  this->Ui->stickyAttributes->setVisible(state);
}

//-----------------------------------------------------------------------------
void vpTrackAttributesPanel::updateEditable()
{
  if (this->Editing)
    {
    this->Ui->attributeTree->setEnabled(true);
    }
  else
    {
    this->Ui->attributeTree->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void vpTrackAttributesPanel::UpdateTrackAttributes(int id)
{
  this->Track = this->Project->TrackModel->GetTrack(id);
  if (!this->Track || !this->Ui->attributeTree->topLevelItemCount())
    {
    this->ShowTrackAttributesControls(false);
    return;
    }
  this->ShowTrackAttributesControls(true);

  vtkSmartPointer<vtkVgScalars> attrScalars =
    this->Track->GetScalars("DetectionAttributes");
  if (!attrScalars)
    {
    attrScalars = vtkSmartPointer<vtkVgScalars>::New();
    attrScalars->SetNotFoundValue(0.0);
    this->Track->SetScalars("DetectionAttributes", attrScalars);
    }

  vtkVgTimeStamp timeStamp;
  timeStamp.SetFrameNumber(this->ViewCoreInstance->getCurrentFrameIndex());
  this->updateEditable();

  if (this->Ui->stickyAttributes->checkState() == Qt::Checked &&
      this->Ui->attributeTree->isEnabled())
    {
    attrScalars->InsertValue(timeStamp, this->GetTrackAttributesValue());
    return;
    }

  vtkTypeUInt64 attrValue = (vtkTypeUInt64)attrScalars->GetValue(timeStamp);
  if (timeStamp < this->Track->GetStartFrame() ||
      timeStamp > this->Track->GetEndFrame())
    {
    // If not sticky and outside bounds of the track, make sure that no
    // attributes are shown
    attrValue = 0;
    }

  bool activeOnly =
    (this->Ui->displayActiveAttributes->checkState() == Qt::Checked);
  for (int topLevelIndex = 0;
       topLevelIndex < this->Ui->attributeTree->topLevelItemCount();
       ++topLevelIndex)
    {
    QTreeWidgetItem* groupItem =
      this->Ui->attributeTree->topLevelItem(topLevelIndex);

    for (int childIndex = 0; childIndex < groupItem->childCount();
         ++childIndex)
      {
      vtkTypeUInt64 mask =
        groupItem->child(childIndex)->data(0, Qt::UserRole).toULongLong();
      groupItem->child(childIndex)->setCheckState(0,
        (attrValue & mask) ? Qt::Checked : Qt::Unchecked);
      groupItem->child(childIndex)->setHidden(activeOnly &&
        groupItem->child(childIndex)->checkState(0) != Qt::Checked);
      }
    }
}

//-----------------------------------------------------------------------------
double vpTrackAttributesPanel::GetTrackAttributesValue()
{
  double attrValue = 0;
  for (int topLevelIndex = 0;
    topLevelIndex < this->Ui->attributeTree->topLevelItemCount();
    ++topLevelIndex)
    {
    QTreeWidgetItem* groupItem =
      this->Ui->attributeTree->topLevelItem(topLevelIndex);

    for (int childIndex = 0; childIndex < groupItem->childCount();
      ++childIndex)
      {
      if (groupItem->child(childIndex)->checkState(0) == Qt::Checked)
        {
        attrValue +=
          groupItem->child(childIndex)->data(0, Qt::UserRole).toULongLong();
        }
      }
    }

  return attrValue;
}

//-----------------------------------------------------------------------------
void vpTrackAttributesPanel::UpdateTrackAttribute(QTreeWidgetItem* attributeItem)
{
  vtkSmartPointer<vtkVgScalars> attrScalars =
    this->Track->GetScalars("DetectionAttributes");
  if (!attrScalars)
    {
    attrScalars = vtkSmartPointer<vtkVgScalars>::New();
    attrScalars->SetNotFoundValue(0.0);
    this->Track->SetScalars("DetectionAttributes", attrScalars);
    }

  vtkVgTimeStamp timeStamp;
  timeStamp.SetFrameNumber(this->ViewCoreInstance->getCurrentFrameIndex());

  // If Editing but outside current bounds of the track, don't set the attribute
  if (this->Editing &&
      (timeStamp < this->Track->GetStartFrame() ||
       timeStamp > this->Track->GetEndFrame()))
    {
    return;
    }
  // If in the bounds of the track but interpolated point, perhaps should not have
  // an attribute

  vtkTypeUInt64 attrValue = (vtkTypeUInt64)attrScalars->GetValue(timeStamp);

  vtkTypeUInt64 mask = attributeItem->data(0, Qt::UserRole).toULongLong();

  // Turn on / off bit value(s) indicated by the mask
  if (attributeItem->checkState(0) == Qt::Checked)
    {
    attrValue |= mask;
    }
  else
    {
    attrValue &= ~mask;
    if (this->Ui->displayActiveAttributes->checkState() == Qt::Checked)
      {
      attributeItem->setHidden(true);
      }
    }

  attrScalars->InsertValue(timeStamp, attrValue);
}

//-----------------------------------------------------------------------------
void vpTrackAttributesPanel::ToggleDisplayOnlyActiveAttributes(int state)
{
  bool activeOnly = (state == Qt::Checked);
  for (int topLevelIndex = 0;
       topLevelIndex < this->Ui->attributeTree->topLevelItemCount();
       ++topLevelIndex)
    {
    QTreeWidgetItem* groupItem =
      this->Ui->attributeTree->topLevelItem(topLevelIndex);

    for (int childIndex = 0; childIndex < groupItem->childCount();
      ++childIndex)
      {
      groupItem->child(childIndex)->setHidden(activeOnly &&
        groupItem->child(childIndex)->checkState(0) != Qt::Checked);
      }
    }
}

//-----------------------------------------------------------------------------
void vpTrackAttributesPanel::BuildAttributeTree()
{
  this->Ui->attributeTree->clear();
  this->Ui->attributeTree->setHeaderHidden(true);

  vgAttributeSet* trackAttributes = &this->Project->TrackDetectionAttributes;
  std::vector<std::string> groups = trackAttributes->GetGroups();
  int numGroups = 0;
  for (size_t i = 0, size = groups.size(); i < size; ++i)
    {
    if (trackAttributes->IsGroupEnabled(groups[i]))
      {
      numGroups++;
      break;
      }
    }
  if (numGroups == 0)
    {
    this->ShowTrackAttributesControls(false);
    return;
    }
  else
    {
    this->ShowTrackAttributesControls(true);
    }

  bool activeOnly =
    (this->Ui->displayActiveAttributes->checkState() == Qt::Checked);

  for (size_t i = 0, size = groups.size(); i < size; ++i)
    {
    if (trackAttributes->IsGroupEnabled(groups[i]))
      {
      QTreeWidgetItem *groupItem = new QTreeWidgetItem();
      groupItem->setText(0, qtString(groups[i]));
      this->Ui->attributeTree->addTopLevelItem(groupItem);

      std::vector<vgAttribute> attrs =
        trackAttributes->GetAttributes(groups[i]);
      for (size_t j = 0; j < attrs.size(); ++j)
        {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, qtString(attrs[j].Name));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, Qt::UserRole, attrs[j].Mask);
        item->setHidden(activeOnly);
        groupItem->addChild(item);
        }
      groupItem->setExpanded(true);
      }
    }
}
