// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "ui_vpObjectInfoPanel.h"

#include "vpObjectInfoPanel.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegExpValidator>

#include <qtScopedValueChange.h>
#include <qtUtil.h>

#include "vgEventType.h"
#include "vgTrackType.h"
#include "vgUnixTime.h"

#include "vpEditTrackTypesDialog.h"
#include "vpTrackConfig.h"
#include "vpTrackIO.h"
#include "vpUtils.h"
#include "vpViewCore.h"
#include "vtkVpTrackModel.h"

#include "vtkVgActivityManager.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventTypeRegistry.h"

#include "vtkVgActivity.h"
#include "vtkVgEvent.h"
#include "vtkVgTrack.h"

//-----------------------------------------------------------------------------
vpObjectInfoPanel::vpObjectInfoPanel(QWidget* p)
  : QWidget(p), Track(0), ChildEvent(0), Editing(false),
    ParentTrackChangesToApply(false), PrevType(-1)
{
  this->Ui = new Ui::vpObjectInfoPanel;
  this->Ui->setupUi(this);

  this->Ui->trackIdEdit->setValidator(new QIntValidator(0, INT_MAX, this));

  this->Ui->trackClassifiers->sortItems(1, Qt::DescendingOrder);

  connect(this->Ui->trackStartSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(StartFrameChanged(int)));

  connect(this->Ui->trackEndSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(EndFrameChanged(int)));

  connect(this->Ui->editProperties, SIGNAL(clicked(bool)),
          this, SLOT(EditProperties()));

  connect(this->Ui->trackType, SIGNAL(activated(int)),
          this, SLOT(UpdateTrackType(int)));

  this->LastStartFrameVal = this->LastEndFrameVal = -1;

  this->ShowEmptyPage();
}

//-----------------------------------------------------------------------------
vpObjectInfoPanel::~vpObjectInfoPanel()
{
  delete this->Ui;
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::ShowEmptyPage()
{
  this->Cleanup();
  this->Ui->editProperties->hide();

  this->Ui->objectInfo->setCurrentWidget(this->Ui->emptyPage);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::Initialize(vpViewCore* viewCore,
                                   vtkVgActivityManager* activityManager,
                                   vtkVgEventModel* eventModel,
                                   vtkVpTrackModel* trackModel,
                                   vtkVgEventTypeRegistry* eventTypes,
                                   vpTrackConfig* trackTypes,
                                   const vpTrackIO* trackIO)
{
  this->ViewCoreInstance = viewCore;
  this->ActivityManager = activityManager;
  this->EventModel = eventModel;
  this->TrackModel = trackModel;
  this->EventTypeRegistry = eventTypes;
  this->TrackConfig = trackTypes;
  this->TrackIO = trackIO;

  this->Editing = false;

  this->BuildTypeList();
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::ShowActivityInfo(int index)
{
  this->Cleanup();
  this->Ui->editProperties->hide();

  vtkVgActivity* a = this->ActivityManager->GetActivity(index);

  this->Ui->activityType->setText(
    this->ActivityManager->GetActivityName(a->GetType()));

  vtkVgTimeStamp startFrame, endFrame;
  a->GetActivityFrameExtents(startFrame, endFrame);

  int frameOffset =
    this->ViewCoreInstance->getUseZeroBasedFrameNumbers() ? 0 : 1;

  this->Ui->typeColor->setVisible(false);

  this->Ui->activityStartFrame->setText(
    vpUtils::GetTimeAndFrameNumberString(startFrame, frameOffset));
  this->Ui->activityEndFrame->setText(
    vpUtils::GetTimeAndFrameNumberString(endFrame, frameOffset));

  this->Ui->activitySaliency->setText(QString::number(a->GetSaliency()));
  this->Ui->activityProbability->setText(QString::number(a->GetProbability()));

  this->Ui->objectInfo->setCurrentWidget(this->Ui->activityPage);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::ShowEventInfo(int id, int /*parentId*/, int /*index*/)
{
  this->Cleanup();
  this->Ui->editProperties->hide();

  vtkVgEvent* e = this->EventModel->GetEvent(id);

  this->Ui->eventType->setText(this->EventTypeRegistry->GetTypeById(
                                 e->GetActiveClassifierType()).GetName());

  this->Ui->eventNormalcy->setText(QString::number(e->GetActiveClassifierNormalcy()));

  vtkVgTimeStamp startFrame = e->GetStartFrame();
  vtkVgTimeStamp endFrame = e->GetEndFrame();

  this->Ui->typeColor->setVisible(false);

  int frameOffset =
    this->ViewCoreInstance->getUseZeroBasedFrameNumbers() ? 0 : 1;

  this->Ui->eventStartFrame->setText(
    vpUtils::GetTimeAndFrameNumberString(startFrame, frameOffset));
  this->Ui->eventEndFrame->setText(
    vpUtils::GetTimeAndFrameNumberString(endFrame, frameOffset));

  this->Ui->objectInfo->setCurrentWidget(this->Ui->eventPage);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::ShowTrackInfo(int id, int parentId, int index)
{
  this->Cleanup();
  this->Ui->editProperties->show();

  this->SetEditProperties(false);

  // use track info from parent event if we have one
  if (parentId >= 0)
    {
    this->ShowParentTrackInfo(id, parentId, index);
    }
  else
    {
    this->Ui->trackGroup->setTitle("Track");
    this->Ui->typeColor->setVisible(true);
    this->Ui->trackClassifiers->setVisible(true);
    this->Ui->trackClassifiersLabel->setVisible(true);
    this->Ui->trackNormalcyLabel->setText("Normalcy:");
    this->ShowTrackInfo(id);
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::ShowSceneElementInfo(int id)
{
  this->Cleanup();
  this->Ui->editProperties->show();

  this->SetEditProperties(false);

  this->Ui->trackGroup->setTitle("Scene Element");
  this->Ui->typeColor->setVisible(true);
  this->Ui->trackClassifiers->setVisible(false);
  this->Ui->trackClassifiersLabel->setVisible(false);
  this->Ui->trackNormalcyLabel->setText("Probability:");
  this->ShowTrackInfo(id);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::ShowTrackInfo(int id)
{
  this->Cleanup();

  vtkVgTrack* t = this->TrackModel->GetTrack(id);
  this->Track = t;

  int displayId = t->GetDisplayFlags() & vtkVgTrack::DF_SceneElement
                    ? this->TrackModel->GetSceneElementIdForTrack(id) : id;

  this->Ui->trackId->setText(QString::number(displayId));
  this->SetCurrentType(t->GetType());
  this->Ui->trackNormalcy->setText(QString::number(t->GetNormalcy()));

  vtkVgTimeStamp startFrame = t->GetStartFrame();
  vtkVgTimeStamp endFrame = t->GetEndFrame();

  QString startTime = vgUnixTime(startFrame.GetTime()).timeString();
  QString endTime = vgUnixTime(endFrame.GetTime()).timeString();

  int frameOffset =
    this->ViewCoreInstance->getUseZeroBasedFrameNumbers() ? 0 : 1;

  this->Ui->trackStartFrame->setText(
    vpUtils::GetTimeAndFrameNumberString(startFrame, frameOffset));
  this->Ui->trackEndFrame->setText(
    vpUtils::GetTimeAndFrameNumberString(endFrame, frameOffset));

  qtDelayTreeSorting ds(this->Ui->trackClassifiers);
  this->Ui->trackClassifiers->clear();

  for (auto c : t->GetTOC())
    {
    auto* const ti = new QTreeWidgetItem{this->Ui->trackClassifiers};
    const auto& type = this->TrackConfig->GetTrackTypeByIndex(c.first);
    ti->setText(0, QString::fromLocal8Bit(type.GetName()));
    ti->setData(1, Qt::DisplayRole, c.second);
    }

  this->Ui->objectInfo->setCurrentWidget(this->Ui->trackPage);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::ShowParentTrackInfo(int id, int parentId, int index)
{
  this->Cleanup();
  this->ParentTrackChangesToApply = false;

  vtkVgEvent* event = this->EventModel->GetEvent(parentId);
  this->ChildEvent = event;

  vtkVgTrack* track;
  vtkVgTimeStamp startFrame, endFrame;
  event->GetTrack(index, track, startFrame, endFrame);

  this->InitialStartFrame = startFrame;
  this->InitialEndFrame = endFrame;

  this->Track = track;
  this->TrackIndex = index;

  this->UpdateParentTrackEventTimes();

  this->Ui->parentTrackId->setText(QString::number(id));

  int frameOffset =
    this->ViewCoreInstance->getUseZeroBasedFrameNumbers() ? 0 : 1;
  int minFrameNum = track->GetStartFrame().GetFrameNumber() + frameOffset;
  int maxFrameNum = track->GetEndFrame().GetFrameNumber() + frameOffset;

  int startVal = startFrame.GetFrameNumber() + frameOffset;
  int endVal = endFrame.GetFrameNumber() + frameOffset;

  this->Ui->trackStartSpinBox->blockSignals(true);
  this->Ui->trackEndSpinBox->blockSignals(true);

  this->Ui->trackStartSpinBox->setMinimum(minFrameNum);
  this->Ui->trackStartSpinBox->setMaximum(std::min(maxFrameNum, endVal));

  this->Ui->trackEndSpinBox->setMinimum(std::max(minFrameNum, startVal));
  this->Ui->trackEndSpinBox->setMaximum(maxFrameNum);

  this->Ui->trackStartSpinBox->setValue(startVal);
  this->Ui->trackEndSpinBox->setValue(endVal);
  this->LastStartFrameVal = startVal;
  this->LastEndFrameVal = endVal;

  this->Ui->trackEndSpinBox->blockSignals(false);
  this->Ui->trackStartSpinBox->blockSignals(false);

  this->Ui->objectInfo->setCurrentWidget(this->Ui->parentTrackPage);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::StartFrameChanged(int val)
{
  int frameOffset =
    this->ViewCoreInstance->getUseZeroBasedFrameNumbers() ? 0 : 1;

  // update the limit of the other spinbox
  int minFrameNum = this->Track->GetStartFrame().GetFrameNumber() + frameOffset;
  this->Ui->trackEndSpinBox->setMinimum(std::max(minFrameNum, val));

  vtkVgTimeStamp timeStamp;
  timeStamp.SetFrameNumber(val -= frameOffset);
  bool advanced = val > (this->LastStartFrameVal - frameOffset);

  // snap the spin box to the closest valid frame
  if (!this->FindClosestValidFrame(timeStamp, advanced))
    {
    this->Ui->trackStartSpinBox->setValue(this->LastStartFrameVal);
    }
  else
    {
    this->LastStartFrameVal = val + frameOffset;
    this->ChildEvent->SetTrackStartFrame(this->TrackIndex, timeStamp);

    if (timeStamp.GetFrameNumber() != static_cast<unsigned>(val))
      {
      this->Ui->trackStartSpinBox->setValue(timeStamp.GetFrameNumber() +
                                            frameOffset);
      }
    this->ParentTrackChangesToApply = true;
    this->UpdateParentTrackEventTimes();
    this->EventModel->Modified();
    this->ViewCoreInstance->UpdateEventModifiedTime();
    this->ViewCoreInstance->updateScene();
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::EndFrameChanged(int val)
{
  int frameOffset =
    this->ViewCoreInstance->getUseZeroBasedFrameNumbers() ? 0 : 1;

  // update the limit of the other spinbox
  int maxFrameNum = this->Track->GetEndFrame().GetFrameNumber() + frameOffset;
  this->Ui->trackStartSpinBox->setMaximum(std::min(maxFrameNum, val));

  vtkVgTimeStamp timeStamp;
  timeStamp.SetFrameNumber(val -= frameOffset);
  bool advanced = val > (this->LastEndFrameVal - frameOffset);

  // snap the spin box to the closest valid frame
  if (!this->FindClosestValidFrame(timeStamp, advanced))
    {
    this->Ui->trackEndSpinBox->setValue(this->LastEndFrameVal);
    }
  else
    {
    this->LastEndFrameVal = val + frameOffset;
    this->ChildEvent->SetTrackEndFrame(this->TrackIndex, timeStamp);

    if (timeStamp.GetFrameNumber() != static_cast<unsigned>(val))
      {
      this->Ui->trackEndSpinBox->setValue(timeStamp.GetFrameNumber() +
                                          frameOffset);
      }

    this->ParentTrackChangesToApply = true;
    this->UpdateParentTrackEventTimes();
    this->EventModel->Modified();
    this->ViewCoreInstance->UpdateEventModifiedTime();
    this->ViewCoreInstance->updateScene();
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::EditProperties()
{
  this->SetEditProperties(!this->Editing);

  QWidget* widget = this->Ui->objectInfo->currentWidget();
  if (widget == this->Ui->trackPage)
    {
    this->EditTrackInfo();
    }
  else if (widget == this->Ui->parentTrackPage)
    {
    this->EditParentTrackInfo();
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::EditTrackInfo()
{
  this->Ui->trackType->setEnabled(this->Editing);

  if (this->Editing)
    {
    this->Ui->trackIdEdit->setText(this->Ui->trackId->text());
    this->Ui->trackIdEdit->selectAll();
    this->Ui->trackIdWidget->setCurrentIndex(1);
    }
  else
    {
    bool ok;
    bool modified = false;
    int id = this->Ui->trackIdEdit->text().toInt(&ok);

    vgObjectTypeDefinitions::enumObjectTypes objectType =
      vgObjectTypeDefinitions::Track;
    if (this->Track->GetDisplayFlags() & vtkVgTrack::DF_SceneElement)
      {
      id = this->TrackModel->GetTrackIdForSceneElement(id);
      objectType = vgObjectTypeDefinitions::SceneElement;
      }

    if (ok && id != this->Track->GetId())
      {
      if (this->TrackModel->GetTrack(id))
        {
        QMessageBox::warning(0, QString(),
                             "A track with this ID already exists.");
        }
      else
        {
        // update the id
        this->TrackModel->SetTrackId(this->Track, id);
        emit this->ObjectIdChanged(objectType, this->Track->GetId());
        modified = true;
        }
      }
    this->Ui->trackIdWidget->setCurrentIndex(0);

    int typeIndex = this->GetCurrentType();

    // update color for the type (if changed)
    if (typeIndex != -1 && this->Ui->typeColor->isVisible())
      {
      int nRGB[3];
      this->Ui->typeColor->color().getRgb(nRGB, nRGB + 1, nRGB + 2);
      if (nRGB[0] != this->TrackTypeRGB[0] ||
          nRGB[1] != this->TrackTypeRGB[1] ||
          nRGB[2] != this->TrackTypeRGB[2])
        {
        // if color for the type changed, we need to change track color
        // of all tracks (which includes scene elements) of this type
        vgTrackType updatedType =
          this->TrackConfig->GetTrackTypeByIndex(typeIndex);
        double rgb[3] = {nRGB[0] / 255.0, nRGB[1] / 255.0, nRGB[2] / 255.0};
        updatedType.SetColor(rgb[0], rgb[1], rgb[2]);
        this->TrackConfig->SetType(typeIndex, updatedType);
        emit this->TypeColorChanged(typeIndex, rgb);
        }
      }

    // update the type
    if (this->Track->GetType() != typeIndex)
      {
      double rgb[3];
      this->Track->SetType(typeIndex);
      if (typeIndex == -1)
        {
        vpTrackIO::GetDefaultTrackColor(this->Track->GetId(), rgb);
        }
      else
        {
        this->TrackConfig->GetTrackTypeByIndex(typeIndex).GetColor(rgb[0],
                                                                   rgb[1],
                                                                   rgb[2]);
        }

      this->Track->SetColor(rgb[0], rgb[1], rgb[2]);

      this->TrackModel->Modified();
      emit this->ObjectTypeChanged(objectType, this->Track->GetId());
      modified = true;
      }

    if (modified)
      {
      if (objectType == vgObjectTypeDefinitions::SceneElement)
        {
        this->ViewCoreInstance->UpdateSceneElementModifiedTime();
        }
      else
        {
        this->ViewCoreInstance->UpdateTrackModifiedTime();
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::EditParentTrackInfo()
{
  if (!this->Editing)
    {
    this->ParentTrackChangesToApply = false;
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::UpdateTrackType(int type)
{
  if (type == this->Ui->trackType->count() - 2)
    {
    // <New..>
    QDialog dlg;
    dlg.setWindowTitle("New Track Type");
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QLineEdit* lineEdit = new QLineEdit;
    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                    QDialogButtonBox::Cancel);

    // Don't allow whitespace or punctuation in the type
    lineEdit->setValidator(new QRegExpValidator(QRegExp("\\w+"), lineEdit));

    layout->addWidget(lineEdit);
    layout->addWidget(btnBox);
    connect(btnBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), &dlg, SLOT(reject()));
    connect(lineEdit, SIGNAL(returnPressed()), &dlg, SLOT(accept()));

    // New track type
    QString label;
    if (dlg.exec() == QDialog::Rejected || (label = lineEdit->text()).isEmpty())
      {
      // User cancelled or didn't enter any text
      this->SetCurrentType(this->Track->GetType());
      return;
      }

    int index = this->TrackConfig->GetTrackTypeIndex(qPrintable(label));
    if (index >= 0 || label == this->Ui->trackType->itemText(0))
      {
      // User typed a string that is an existing track type
      this->SetCurrentType(index);
      }
    else
      {
      // Add the new type
      vgTrackType tt;
      tt.SetId(qPrintable(label));
      this->TrackConfig->AddType(tt);
      type = this->GetCurrentType();
      // Rebuild the combo box
      this->BuildTypeList();
      // Set the combo box to the new type
      this->Ui->trackType->setCurrentIndex(
        this->Ui->trackType->findText(label));
      }
    }
  else if (type == this->Ui->trackType->count() - 1)
    {
    // <Edit..>
    vpEditTrackTypesDialog dlg(this->TrackConfig, this->TrackModel, this);
    dlg.exec();
    this->BuildTypeList();
    this->SetCurrentType(this->PrevType);
    }
  else
    {
    this->PrevType = this->GetCurrentType();
    int index = type - 1;
    if (index == -1)
      {
      this->Ui->typeColor->setVisible(false);
      }
    else
      {
      double rgb[3];
      this->Ui->typeColor->setVisible(true);
      this->TrackConfig->GetTrackTypeByIndex(index).GetColor(rgb[0],
                                                             rgb[1],
                                                             rgb[2]);
      this->TrackTypeRGB[0] = 255.0 * rgb[0];
      this->TrackTypeRGB[1] = 255.0 * rgb[1];
      this->TrackTypeRGB[2] = 255.0 * rgb[2];
      this->Ui->typeColor->setColor(QColor(this->TrackTypeRGB[0],
                                           this->TrackTypeRGB[1],
                                           this->TrackTypeRGB[2]));
      }
    }
}

//-----------------------------------------------------------------------------
bool vpObjectInfoPanel::FindClosestValidFrame(vtkVgTimeStamp& current,
                                              bool next)
{
  // HACK: Ask the reader as well as the track itself for the closest frame. If
  // both agree on the frame number, use the reader timestamp since it is more
  // likely to contain both time and frame. We can't just always use the reader
  // frame, since the reader does not get updated when adding or removing points
  // from tracks. And we can't just use the track timestamp without potentially
  // throwing out time info, since we currently use frame number only when
  // setting the initial track points.
  vtkVgTimeStamp readerFrame;
  bool valid;

  if (next)
    {
    this->TrackIO->GetNextValidTrackFrame(this->Track,
                                          current.GetFrameNumber(),
                                          readerFrame);
    valid = this->Track->GetFrameAtOrAfter(current);
    }
  else
    {
    this->TrackIO->GetPrevValidTrackFrame(this->Track,
                                          current.GetFrameNumber(),
                                          readerFrame);
    valid = this->Track->GetFrameAtOrBefore(current);
    }

  if (valid && readerFrame.IsValid() &&
      current.GetFrameNumber() == readerFrame.GetFrameNumber())
    {
    current = readerFrame;
    return true;
    }

  return valid;
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::UpdateParentTrackEventTimes()
{
  vtkVgTrack* track;
  vtkVgTimeStamp startFrame, endFrame;
  this->ChildEvent->GetTrack(this->TrackIndex, track, startFrame, endFrame);

  QString startTime = vgUnixTime(startFrame.GetTime()).timeString();
  QString endTime = vgUnixTime(endFrame.GetTime()).timeString();

  this->Ui->parentTrackEventStartTime->setText(startTime);
  this->Ui->parentTrackEventEndTime->setText(endTime);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::SetEditProperties(bool edit)
{
  this->Editing = edit;
  this->Ui->trackType->setEnabled(edit);
  this->Ui->typeColor->setEnabled(edit);
  this->Ui->trackStartSpinBox->setEnabled(edit);
  this->Ui->trackEndSpinBox->setEnabled(edit);

  if (edit)
    {
    this->Ui->editProperties->setIcon(qtUtil::standardIcon("apply", 16));
    this->Ui->editProperties->setToolTip("Apply Changes");

    this->Ui->trackIdEdit->setText(this->Ui->trackId->text());
    this->Ui->trackIdWidget->setCurrentIndex(1);
    this->Ui->trackIdEdit->setFocus();
    this->Ui->trackIdEdit->selectAll();
    }
  else
    {
    this->Ui->editProperties->setIcon(qtUtil::standardIcon("configure", 16));
    this->Ui->editProperties->setToolTip("Modify");
    this->Ui->trackIdWidget->setCurrentIndex(0);
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::Cleanup()
{
  if (this->Ui->objectInfo->currentWidget() == this->Ui->parentTrackPage)
    {
    // If the user switched away without applying changes, restore the original
    // state of the child event.
    if (this->ParentTrackChangesToApply)
      {
      this->ParentTrackChangesToApply = false;
      this->ChildEvent->SetTrackStartFrame(this->TrackIndex,
                                           this->InitialStartFrame);
      this->ChildEvent->SetTrackEndFrame(this->TrackIndex,
                                         this->InitialEndFrame);
      this->EventModel->Modified();
      this->ViewCoreInstance->updateScene();
      }
    }
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::BuildTypeList()
{
  this->Ui->trackType->clear();
  this->Ui->trackType->addItem("None", -1);

  for (int i = 0, end = this->TrackConfig->GetNumberOfTypes(); i < end; ++i)
    {
    const char* name = this->TrackConfig->GetTrackTypeByIndex(i).GetName();
    if (*name)
      {
      this->Ui->trackType->addItem(name, i);
      }
    }

  this->Ui->trackType->insertSeparator(this->Ui->trackType->count());

  this->Ui->trackType->addItem("<New...>", -1);
  this->Ui->trackType->addItem("<Edit...>", -1);
}

//-----------------------------------------------------------------------------
void vpObjectInfoPanel::SetCurrentType(int index)
{
  this->PrevType = index;
  int cbIndex = this->Ui->trackType->findData(index);
  this->Ui->trackType->setCurrentIndex(cbIndex == -1 ? 0 : cbIndex);
  double rgb[3];
  if (index == -1 || cbIndex == 0)
    {
    this->Ui->typeColor->setVisible(false);
    }
  else
    {
    this->Ui->typeColor->setVisible(true);
    this->TrackConfig->GetTrackTypeByIndex(index).GetColor(rgb[0], rgb[1], rgb[2]);
    this->TrackTypeRGB[0] = 255.0 * rgb[0];
    this->TrackTypeRGB[1] = 255.0 * rgb[1];
    this->TrackTypeRGB[2] = 255.0 * rgb[2];
    this->Ui->typeColor->setColor(QColor(this->TrackTypeRGB[0],
                                         this->TrackTypeRGB[1],
                                         this->TrackTypeRGB[2]));
    }
}

//-----------------------------------------------------------------------------
int vpObjectInfoPanel::GetCurrentType()
{
  return this->Ui->trackType->itemData(
           this->Ui->trackType->currentIndex()).toInt();
}
