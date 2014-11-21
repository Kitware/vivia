/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpCreateEventDialog.h"

#include "ui_vpCreateEventDialog.h"

#include "vpUtils.h"
#include "vpViewCore.h"

#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>

#include <vtkIdList.h>

//-----------------------------------------------------------------------------
class vpCreateEventDialogPrivate
{
public:
  Ui_vpCreateEventDialog UI;

  int Session;

  vpViewCore* ViewCore;
  vtkSmartPointer<vtkVgEvent> Event;
  vtkSmartPointer<vtkVgEventModel> EventModel;
};

QTE_IMPLEMENT_D_FUNC(vpCreateEventDialog)

//-----------------------------------------------------------------------------
vpCreateEventDialog::vpCreateEventDialog(vpViewCore* coreInstance, int session,
                                         QWidget* parent,
                                         Qt::WindowFlags flags)
  : QDialog(parent, flags), d_ptr(new vpCreateEventDialogPrivate)
{
  QTE_D(vpCreateEventDialog);

  d->UI.setupUi(this);

  d->Session = session;
  d->ViewCore = coreInstance;
  d->EventModel = coreInstance->getEventModel(session);

  // populate the event types combo
  vtkVgEventTypeRegistry* eventTypes = d->ViewCore->getEventTypeRegistry();
  for (int i = 0, end = eventTypes->GetNumberOfTypes(); i != end; ++i)
    {
    const vgEventType& type = eventTypes->GetType(i);
    d->UI.comboBox->addItem(type.GetName(), type.GetId());
    }

  d->UI.status->setVisible(false);
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setText("Add Event");
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
  d->UI.buttonBox->button(QDialogButtonBox::Cancel)->setText("Close");

  // signal-slot connections
  connect(d->ViewCore, SIGNAL(trackPicked(int, int)),
          this, SLOT(addTrack(int, int)));

  connect(d->UI.comboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setEventType(int)));

  connect(d->UI.startHere, SIGNAL(clicked()), this, SLOT(startEventHere()));
  connect(d->UI.endHere, SIGNAL(clicked()), this, SLOT(endEventHere()));

  connect(d->UI.trackUp, SIGNAL(clicked()), this, SLOT(moveTrackUp()));
  connect(d->UI.trackDown, SIGNAL(clicked()), this, SLOT(moveTrackDown()));
  connect(d->UI.trackDelete, SIGNAL(clicked()), this, SLOT(removeTrack()));
}

//-----------------------------------------------------------------------------
vpCreateEventDialog::~vpCreateEventDialog()
{
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::initialize()
{
  QTE_D(vpCreateEventDialog);

  // create the event we will be showing in the main view
  vtkSmartPointer<vtkIdList> trackIds = vtkSmartPointer<vtkIdList>::New();
  int id = d->ViewCore->createEvent(
             d->UI.comboBox->itemData(d->UI.comboBox->currentIndex()).toInt(),
             trackIds, d->Session);

  d->Event = d->EventModel->GetEvent(id);

  if (d->Event)
    {
    d->Event->SetStartFrame(d->ViewCore->getCoreTimeStamp());
    }

  d->UI.trackList->clear();
  this->updateEventTimeRange();
  this->validate();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::addTrack(int trackId, int session)
{
  QTE_D(vpCreateEventDialog);

  if (!d->Event || session != d->Session)
    {
    return;
    }

  vtkVgTrack* track = d->EventModel->GetTrackModel()->GetTrack(trackId);

  vtkVgTimeStamp trackStart = track->GetStartFrame();
  vtkVgTimeStamp trackEnd = track->GetEndFrame();

  // look for existing track with that id
  QTreeWidgetItem* item = 0;
  for (int i = 0, end = d->UI.trackList->topLevelItemCount(); i != end; ++i)
    {
    QTreeWidgetItem* current = d->UI.trackList->topLevelItem(i);
    if (current->data(0, Qt::DisplayRole).toInt() == trackId)
      {
      item = current;
      break;
      }
    }

  // create new item if not already present
  if (!item)
    {
    item = new QTreeWidgetItem;
    item->setData(0, Qt::DisplayRole, trackId);
    d->UI.trackList->addTopLevelItem(item);

    // If the event start or end time haven't been defined, set the track start
    // or end to the current frame. Otherwise set it to the event start or end
    // time. This allows a user to rapidly create events by creating the event
    // when the main view is on the desired start frame, then advancing to the
    // last frame and clicking all the tracks that should be included.
    vtkVgTimeStamp frame = d->ViewCore->getCoreTimeStamp();
    vtkVgTimeStamp etStart = d->Event->GetStartFrame().IsMaxTime() ? frame :
                             d->Event->GetStartFrame();
    vtkVgTimeStamp etEnd = d->Event->GetEndFrame().IsMinTime() ? frame :
                           d->Event->GetEndFrame();

    // Make sure the end frame is not before the start. This may happen if the
    // dialog is initialized, then the frame is moved back before a track is
    // added.
    if (etStart > etEnd)
      {
      etStart = etEnd;
      }

    d->Event->AddTrack(
      track,
      this->clamp(etStart, trackStart, trackEnd),
      this->clamp(etEnd, trackStart, trackEnd));
    }

  // select it
  d->UI.trackList->setCurrentItem(item);

  this->updateEventTimeRange();
  this->updateTrackTimeRanges();
  this->validate();

  d->EventModel->Modified();
  d->ViewCore->update();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::accept()
{
  QTE_D(vpCreateEventDialog);

  // "finalize" the event and we're done
  d->Event->ClearFlags(vtkVgEvent::EF_Modifiable);

  vtkIdType id = d->Event->GetId();
  emit this->eventCreated(id);

  this->initialize();
  d->UI.status->setText(QString("(created event %1)").arg(id));
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::reject()
{
  QTE_D(vpCreateEventDialog);

  // remove our temp event
  if (d->Event)
    {
    d->EventModel->RemoveEvent(d->Event->GetId());
    d->ViewCore->update();
    }

  this->QDialog::reject();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::setEventType(int typeIndex)
{
  QTE_D(vpCreateEventDialog);

  this->validate();

  if (d->Event)
    {
    d->Event->ResetClassifiers();
    d->Event->AddClassifier(d->UI.comboBox->itemData(typeIndex).toInt(), 1.0);
    d->EventModel->Modified();
    d->ViewCore->update();
    }
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::startEventHere()
{
  QTE_D(vpCreateEventDialog);

  if (!d->Event)
    {
    return;
    }

  vtkVgTimeStamp frame = d->ViewCore->getCoreTimeStamp();
  vtkVgTimeStamp endFrame = d->Event->GetEndFrame();

  // If there are no tracks yet, set the start time on event directy. Otherwise,
  // let the event adjust it's own time range based on the range of the parent
  // tracks.
  if (d->UI.trackList->topLevelItemCount() == 0)
    {
    if (!endFrame.IsMinTime() && endFrame < frame)
      {
      d->Event->SetEndFrame(frame);
      }
    d->Event->SetStartFrame(frame);
    }
  else
    {
    // propagate start time to tracks
    for (int i = 0, size = d->UI.trackList->topLevelItemCount(); i < size; ++i)
      {
      // don't go outside the bounds of the full track
      vtkVgTrack* track = d->Event->GetTrack(i);
      vtkVgTimeStamp ts = this->clamp(frame, track->GetStartFrame(),
                                      track->GetEndFrame());

      if (ts > d->Event->GetTrackInfo(i)->EndFrame)
        {
        d->Event->SetTrackEndFrame(i, ts);
        }
      d->Event->SetTrackStartFrame(i, ts);
      }
    }

  this->updateEventTimeRange();
  this->updateTrackTimeRanges();

  d->EventModel->Modified();
  d->ViewCore->update();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::endEventHere()
{
  QTE_D(vpCreateEventDialog);

  if (!d->Event)
    {
    return;
    }

  vtkVgTimeStamp frame = d->ViewCore->getCoreTimeStamp();
  vtkVgTimeStamp startFrame = d->Event->GetStartFrame();

  if (d->UI.trackList->topLevelItemCount() == 0)
    {
    if (!startFrame.IsMaxTime() && startFrame > frame)
      {
      d->Event->SetStartFrame(frame);
      }
    d->Event->SetEndFrame(frame);
    }
  else
    {
    // propagate end time to tracks
    for (int i = 0, size = d->UI.trackList->topLevelItemCount(); i < size; ++i)
      {
      // don't go outside the bounds of the full track
      vtkVgTrack* track = d->Event->GetTrack(i);
      vtkVgTimeStamp ts = this->clamp(frame, track->GetStartFrame(),
                                      track->GetEndFrame());

      if (ts < d->Event->GetTrackInfo(i)->StartFrame)
        {
        d->Event->SetTrackStartFrame(i, ts);
        }
      d->Event->SetTrackEndFrame(i, ts);
      }
    }

  this->updateEventTimeRange();
  this->updateTrackTimeRanges();

  d->EventModel->Modified();
  d->ViewCore->update();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::moveTrackUp()
{
  QTE_D(vpCreateEventDialog);

  QModelIndex index = d->UI.trackList->currentIndex();
  if (!index.isValid() || index.row() == 0)
    {
    return;
    }

  d->Event->SwapTracks(index.row(), index.row() - 1);

  QTreeWidgetItem* item = d->UI.trackList->takeTopLevelItem(index.row());
  d->UI.trackList->insertTopLevelItem(index.row() - 1, item);
  d->UI.trackList->setCurrentItem(item);

  d->EventModel->Modified();
  d->ViewCore->update();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::moveTrackDown()
{
  QTE_D(vpCreateEventDialog);

  QModelIndex index = d->UI.trackList->currentIndex();
  if (!index.isValid() ||
      index.row() == d->UI.trackList->model()->rowCount() - 1)
    {
    return;
    }

  d->Event->SwapTracks(index.row(), index.row() + 1);

  QTreeWidgetItem* item = d->UI.trackList->takeTopLevelItem(index.row());
  d->UI.trackList->insertTopLevelItem(index.row() + 1, item);
  d->UI.trackList->setCurrentItem(item);

  d->EventModel->Modified();
  d->ViewCore->update();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::removeTrack()
{
  QTE_D(vpCreateEventDialog);

  QModelIndex index = d->UI.trackList->currentIndex();
  if (!index.isValid())
    {
    return;
    }

  d->Event->RemoveTrack(index.row());
  delete d->UI.trackList->takeTopLevelItem(index.row());
  this->validate();
  d->EventModel->Modified();
  d->ViewCore->update();
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::updateEventTimeRange()
{
  QTE_D(vpCreateEventDialog);

  if (!d->Event)
    {
    d->UI.startTime->setText(QString());
    d->UI.endTime->setText(QString());
    return;
    }

  int frameOffset = d->ViewCore->getUseZeroBasedFrameNumbers() ? 0 : 1;
  d->UI.startTime->setText(
    vpUtils::GetTimeAndFrameNumberString(d->Event->GetStartFrame(), frameOffset));
  d->UI.endTime->setText(
    vpUtils::GetTimeAndFrameNumberString(d->Event->GetEndFrame(), frameOffset));
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::updateTrackTimeRanges()
{
  QTE_D(vpCreateEventDialog);

  if (!d->Event)
    {
    return;
    }

  int frameOffset = d->ViewCore->getUseZeroBasedFrameNumbers() ? 0 : 1;

  for (int i = 0, size = d->UI.trackList->topLevelItemCount(); i < size; ++i)
    {
    QTreeWidgetItem* item = d->UI.trackList->topLevelItem(i);
    vtkVgEventTrackInfoBase* info = d->Event->GetTrackInfo(i);
    item->setText(1, vpUtils::GetTimeAndFrameNumberString(info->StartFrame,
                                                          frameOffset));
    item->setText(2, vpUtils::GetTimeAndFrameNumberString(info->EndFrame,
                                                          frameOffset));
    }
}

//-----------------------------------------------------------------------------
void vpCreateEventDialog::validate()
{
  QTE_D(vpCreateEventDialog);

  int typeIndex = d->UI.comboBox->currentIndex();
  if (d->UI.comboBox->count() == 0 || typeIndex < 0)
    {
    d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    d->UI.status->setText("(valid event type not selected)");
    d->UI.status->setVisible(true);
    return;
    }

  if (!d->Event)
    {
    d->UI.status->setText("(failed to create placeholder event)");
    d->UI.status->setVisible(true);
    return;
    }

  int minTracks =
    d->ViewCore->getEventTypeRegistry()->GetType(typeIndex).GetMinTracks();
  int maxTracks =
    d->ViewCore->getEventTypeRegistry()->GetType(typeIndex).GetMaxTracks();

  int numTracks = d->Event->GetNumberOfTracks();

  if (numTracks < minTracks)
    {
    d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    d->UI.status->setText("(not enough tracks for this type)");
    d->UI.status->setVisible(true);
    return;
    }

  if (maxTracks > 0 && numTracks > maxTracks)
    {
    d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    d->UI.status->setText("(too many tracks for this type)");
    d->UI.status->setVisible(true);
    return;
    }

  // all good
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  d->UI.status->setVisible(false);
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpCreateEventDialog::clamp(const vtkVgTimeStamp& val,
                                          const vtkVgTimeStamp& a,
                                          const vtkVgTimeStamp& b)
{
  return val < a ? a : val > b ? b : val;
}
