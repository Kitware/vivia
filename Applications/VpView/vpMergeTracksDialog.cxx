/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpMergeTracksDialog.h"

#include "ui_vpMergeTracksDialog.h"

#include "vpUtils.h"
#include "vpViewCore.h"

#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>

#include <QCheckBox>
#include <QDebug>
#include <QDialog>
#include <QIntValidator>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>

#include <algorithm>

const char* ExtendEventsKey = "AutoExtendEvents";
const char* MergeEventsKey = "AutoMergeEvents";
const char* MergeGapKey = "AutoMergeEventsMaxGap";

//-----------------------------------------------------------------------------
class vpMergeTracksDialogPrivate
{
public:
  Ui_vpMergeTracksDialog UI;

  int Session;

  vpViewCore* ViewCore;
  vtkSmartPointer<vtkVgTrackModel> TrackModel;

  bool AutoExtendEvents;
  bool AutoMergeEvents;
  int  MaxMergeGap;
};

QTE_IMPLEMENT_D_FUNC(vpMergeTracksDialog)

//-----------------------------------------------------------------------------
vpMergeTracksDialog::vpMergeTracksDialog(vpViewCore* coreInstance, int session,
                                         QWidget* parent,
                                         Qt::WindowFlags flags) :
  QDialog(parent, flags), d_ptr(new vpMergeTracksDialogPrivate)
{
  QTE_D(vpMergeTracksDialog);

  d->UI.setupUi(this);

  d->UI.trackId->setValidator(new QIntValidator(0, INT_MAX, this));

  d->Session = session;
  d->ViewCore = coreInstance;
  d->TrackModel = coreInstance->getTrackModel(session);

  d->UI.status->setVisible(false);
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setText("Add Track");
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

  // signal-slot connections
  connect(d->ViewCore, SIGNAL(trackPicked(int, int)),
          this, SLOT(addTrack(int, int)));

  connect(d->UI.trackId, SIGNAL(textEdited(QString)), this, SLOT(validate()));

  connect(d->UI.replaceMergedTracks, SIGNAL(toggled(bool)),
          this, SLOT(changeMergeType(bool)));

  connect(d->UI.trackUp, SIGNAL(clicked()), this, SLOT(moveTrackUp()));
  connect(d->UI.trackDown, SIGNAL(clicked()), this, SLOT(moveTrackDown()));
  connect(d->UI.trackDelete, SIGNAL(clicked()), this, SLOT(removeTrack()));

  connect(d->UI.advancedOptions, SIGNAL(clicked()),
          this, SLOT(showAdvancedOptions()));
}

//-----------------------------------------------------------------------------
vpMergeTracksDialog::~vpMergeTracksDialog()
{
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::initialize()
{
  QTE_D(vpMergeTracksDialog);

  d->UI.trackList->clear();

  this->updateOptionSettings();

  d->UI.trackId->setText(
    QString::number(d->ViewCore->getCreateTrackId(d->Session)));

  d->UI.trackId->selectAll();
  d->UI.trackId->setFocus();

  this->validate();
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::addTrack(int trackId, int session)
{
  QTE_D(vpMergeTracksDialog);

  if (session != d->Session)
    {
    return;
    }

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
    }

  // select it
  d->UI.trackList->setCurrentItem(item);

  this->updateTrackTimeRanges();
  this->validate();
}

namespace
{

//-----------------------------------------------------------------------------
struct EventIdLessThan
{
  bool operator()(vtkVgEvent* a, vtkVgEvent* b)
    {
    return a->GetId() < b->GetId();
    }
};

//-----------------------------------------------------------------------------
struct EventIdEqual
{
  bool operator()(vtkVgEvent* a, vtkVgEvent* b)
    {
    return a->GetId() == b->GetId();
    }
};

//-----------------------------------------------------------------------------
struct EventStartsBefore
{
  bool operator()(vtkVgEvent* a, vtkVgEvent* b)
    {
    return a->GetStartFrame() < b->GetStartFrame();
    }
};

}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::accept()
{
  QTE_D(vpMergeTracksDialog);

  if (d->AutoExtendEvents && d->UI.trackList->topLevelItemCount() > 2)
    {
    bool isForward = true;
    bool isBackward = true;
    for (int i = 1, end = d->UI.trackList->topLevelItemCount(); i < end; ++i)
      {
      if (d->TrackModel->GetTrack(this->itemTrackId(i))->GetStartFrame() <
          d->TrackModel->GetTrack(this->itemTrackId(i - 1))->GetStartFrame())
        {
        isForward = false;
        }
      if (d->TrackModel->GetTrack(this->itemTrackId(i))->GetEndFrame() >
          d->TrackModel->GetTrack(this->itemTrackId(i - 1))->GetEndFrame())
        {
        isBackward = false;
        }
      }

    if (!(isForward || isBackward))
      {
      if (QMessageBox::warning(this, "Continue?",
                               "Extending events when merging unordered tracks "
                               "may have unexpected results.\n\nContinue?",
                               QMessageBox::Ok | QMessageBox::Cancel) !=
            QMessageBox::Ok)
        {
        return;
        }
      }
    }

  // Issue a warning if events that are already on the same track will be
  // merged.
  if (d->AutoMergeEvents)
    {
    std::vector<vtkVgEvent*> events;
    for (int i = 0, end = d->UI.trackList->topLevelItemCount(); i < end; ++i)
      {
      int trackId = this->itemTrackId(i);

      events.clear();
      d->ViewCore->getEventModel(d->Session)->GetEvents(trackId, events);
      if (this->mergeEvents(events, true))
        {
        if (QMessageBox::warning(this, "Continue?",
                                 "This operation will cause two or more "
                                 "events that are already on the same track "
                                 "to be merged.\n\n"
                                 "Continue?",
                                 QMessageBox::Ok | QMessageBox::Cancel) !=
            QMessageBox::Ok)
          {
          return;
          }

        // proceed with merge
        break;
        }
      }
    }

  if (d->UI.replaceMergedTracks->isChecked())
    {
    // Destructive merge
    std::vector<vtkVgEvent*> events, mergedTrackEvents;
    int mergeId = this->itemTrackId(0);

    vtkVgTrack* mergedTrack = d->TrackModel->GetTrack(mergeId);

    for (int i = 1, end = d->UI.trackList->topLevelItemCount(); i < end; ++i)
      {
      int thisId = this->itemTrackId(i);
      vtkVgTrack* track = d->TrackModel->GetTrack(thisId);

      vtkVgTimeStamp mergedPrevStart = mergedTrack->GetStartFrame();
      vtkVgTimeStamp mergedPrevEnd = mergedTrack->GetEndFrame();

      vtkVgTimeStamp prevStart = track->GetStartFrame();
      vtkVgTimeStamp prevEnd = track->GetEndFrame();

      if (!d->ViewCore->mergeTracks(mergeId, thisId, d->Session))
        {
        d->UI.status->setText(QString("(merge failed)"));
        d->UI.status->setVisible(true);
        return;
        }

      // Try to extend events that reference the merged track
      if (d->AutoExtendEvents)
        {
        d->ViewCore->getEventModel(d->Session)->GetEvents(mergeId,
                                                          mergedTrackEvents);
        for (size_t i = 0, end = mergedTrackEvents.size(); i < end; ++i)
          {
          this->extendEventTrack(mergedTrackEvents[i], mergeId,
                                 mergedPrevStart, mergedPrevEnd,
                                 prevEnd, prevStart);
          }
        mergedTrackEvents.clear();
        }

      // Update any events that reference the track that will be going away
      d->ViewCore->getEventModel(d->Session)->GetEvents(thisId, events);
      for (size_t i = 0, end = events.size(); i < end; ++i)
        {
        this->fixTrackReferences(events[i], thisId, mergeId);

        if (d->AutoExtendEvents)
          {
          this->extendEventTrack(events[i], mergeId, track->GetStartFrame(),
                                 track->GetEndFrame(),
                                 mergedPrevEnd, mergedPrevStart);
          }
        }
      events.clear();

      d->ViewCore->deleteTrack(thisId, d->Session);
      }

    // Merge events where possible
    if (d->AutoMergeEvents)
      {
      d->ViewCore->getEventModel(d->Session)->GetEvents(mergeId,
                                                        mergedTrackEvents);
      this->mergeEvents(mergedTrackEvents);
      }

    emit this->tracksMerged(mergeId);

    this->initialize();
    d->UI.status->setText(QString("(merged tracks into track %1)").arg(mergeId));
    d->UI.status->setVisible(true);

    return;
    }

  // Non-destructive merge
  std::vector<vtkVgEvent*> events;
  for (int i = 0, end = d->UI.trackList->topLevelItemCount(); i < end; ++i)
    {
    int thisId = this->itemTrackId(i);
    d->ViewCore->getEventModel(d->Session)->GetEvents(thisId, events);
    }

  bool createNewEvents = false;
  if (!events.empty())
    {
    QMessageBox::StandardButton btn =
      QMessageBox::question(this, "Create New Events?",
                            "A track to be merged is referenced in one or more "
                            "events. Do you wish to create new events?",
                            QMessageBox::Yes | QMessageBox::No |
                            QMessageBox::Cancel);

    if (btn == QMessageBox::Cancel || btn == QMessageBox::Escape)
      {
      return;
      }
    createNewEvents = btn == QMessageBox::Yes;
    }

  int mergeId = d->UI.trackId->text().toInt();

  vtkVgTrack* track =
    d->ViewCore->cloneTrack(this->itemTrackId(0), mergeId, d->Session);

  if (!track)
    {
    d->UI.status->setText(QString("(failed to create track)"));
    d->UI.status->setVisible(true);
    return;
    }

  d->ViewCore->setCreateTrackId(mergeId + 1, d->Session);

  for (int i = 1, end = d->UI.trackList->topLevelItemCount(); i < end; ++i)
    {
    if (!d->ViewCore->mergeTracks(mergeId, this->itemTrackId(i), d->Session))
      {
      d->UI.status->setText(QString("(merge failed)"));
      d->UI.status->setVisible(true);
      return;
      }
    }

  int numEventsCreated = 0;
  if (createNewEvents)
    {
    // same event may be referenced by multiple tracks, so unique the vector
    std::sort(events.begin(), events.end(), EventIdLessThan());
    std::vector<vtkVgEvent*>::iterator newEnd =
      std::unique(events.begin(), events.end(), EventIdEqual());
    events.resize(newEnd - events.begin());

    std::vector<vtkVgEvent*> newEvents;
    newEvents.reserve(events.size());

    bool failed = false;
    for (size_t i = 0, end = events.size(); i < end; ++i)
      {
      vtkVgEvent* event =
        d->ViewCore->cloneEvent(events[i]->GetId(), d->Session);
      if (!event)
        {
        failed = true;
        continue;
        }
      newEvents.push_back(event);

      // fix up references any of the merged tracks in the new event
      vtkVgTimeStamp mergedPrevStart, mergedPrevEnd;
      for (int i = 0, end = d->UI.trackList->topLevelItemCount(); i < end; ++i)
        {
        vtkVgTrack* thisTrack = d->TrackModel->GetTrack(this->itemTrackId(i));

        // Simulate the iterative extension of the track when extending events.
        // We want to get the same extension of events that happens when merging
        // the tracks in order, two at a time.
        if (i == 0)
          {
          mergedPrevStart = thisTrack->GetStartFrame();
          mergedPrevEnd = thisTrack->GetEndFrame();
          }

        vtkIdType trackId = event->GetTrack(0)->GetId();
        if (trackId == thisTrack->GetId())
          {
          if (d->AutoExtendEvents)
            {
            // event track getting extended for the first time
            this->extendEventTrack(event, trackId,
                                   thisTrack->GetStartFrame(),
                                   thisTrack->GetEndFrame(),
                                   mergedPrevEnd, mergedPrevStart);
            }

          this->fixTrackReferences(event, thisTrack->GetId(), mergeId);
          }
        else if (d->AutoExtendEvents && trackId == mergeId)
          {
          // event track is getting extended again
          this->extendEventTrack(event, mergeId,
                                 mergedPrevStart, mergedPrevEnd,
                                 thisTrack->GetEndFrame(),
                                 thisTrack->GetStartFrame());
          }

        // "Merge" track i and i - 1
        if (thisTrack->GetStartFrame() < mergedPrevStart)
          {
          mergedPrevStart = thisTrack->GetStartFrame();
          }
        if (thisTrack->GetEndFrame() > mergedPrevEnd)
          {
          mergedPrevEnd = thisTrack->GetEndFrame();
          }
        }
      ++numEventsCreated;
      }

    if (failed)
      {
      QMessageBox::warning(this, "Event Cloning Failed",
                           "Failed to clone one or more events");
      }
    else if (d->AutoMergeEvents)
      {
      // merge new events where possible
      this->mergeEvents(newEvents);
      }
    }

  emit this->tracksMerged(mergeId);

  this->initialize();
  if (createNewEvents)
    {
    d->UI.status->setText(QString("(created track %1 and %2 new events)")
                          .arg(mergeId).arg(numEventsCreated));
    }
  else
    {
    d->UI.status->setText(QString("(created track %1)").arg(mergeId));
    }

  d->UI.status->setVisible(true);
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::changeMergeType(bool replaceMerged)
{
  QTE_D(vpMergeTracksDialog);

  d->UI.trackId->setEnabled(!replaceMerged);
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setText(
    replaceMerged ? "Merge Tracks" : "Add Track");
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::moveTrackUp()
{
  QTE_D(vpMergeTracksDialog);

  QModelIndex index = d->UI.trackList->currentIndex();
  if (!index.isValid() || index.row() == 0)
    {
    return;
    }

  QTreeWidgetItem* item = d->UI.trackList->takeTopLevelItem(index.row());
  d->UI.trackList->insertTopLevelItem(index.row() - 1, item);
  d->UI.trackList->setCurrentItem(item);
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::moveTrackDown()
{
  QTE_D(vpMergeTracksDialog);

  QModelIndex index = d->UI.trackList->currentIndex();
  if (!index.isValid() ||
      index.row() == d->UI.trackList->model()->rowCount() - 1)
    {
    return;
    }

  QTreeWidgetItem* item = d->UI.trackList->takeTopLevelItem(index.row());
  d->UI.trackList->insertTopLevelItem(index.row() + 1, item);
  d->UI.trackList->setCurrentItem(item);
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::removeTrack()
{
  QTE_D(vpMergeTracksDialog);

  QModelIndex index = d->UI.trackList->currentIndex();
  if (!index.isValid())
    {
    return;
    }

  delete d->UI.trackList->takeTopLevelItem(index.row());

  this->validate();
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::updateTrackTimeRanges()
{
  QTE_D(vpMergeTracksDialog);

  int frameOffset = d->ViewCore->getUseZeroBasedFrameNumbers() ? 0 : 1;

  for (int i = 0, size = d->UI.trackList->topLevelItemCount(); i < size; ++i)
    {
    QTreeWidgetItem* item = d->UI.trackList->topLevelItem(i);
    vtkVgTrack* track =
      d->TrackModel->GetTrack(item->data(0, Qt::DisplayRole).toInt());

    item->setText(1,
      vpUtils::GetTimeAndFrameNumberString(track->GetStartFrame(), frameOffset));

    item->setText(2,
      vpUtils::GetTimeAndFrameNumberString(track->GetEndFrame(), frameOffset));
    }
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::updateOptionSettings()
{
  QTE_D(vpMergeTracksDialog);

  QSettings settings;
  settings.beginGroup("Annotation");

  d->AutoExtendEvents = settings.value(ExtendEventsKey, false).toBool();
  d->AutoMergeEvents = settings.value(MergeEventsKey, true).toBool();
  d->MaxMergeGap = settings.value(MergeGapKey, 2).toInt();

  if (!d->AutoExtendEvents && !d->AutoMergeEvents)
    {
    d->UI.advancedOptionsLabel->clear();
    return;
    }

  QString label;
  if (d->AutoExtendEvents)
    {
    label += 'E';
    }
  if (d->AutoMergeEvents)
    {
    label += 'M';
    label += QString::number(d->MaxMergeGap);
    }

  d->UI.advancedOptionsLabel->setText(label);
}

//-----------------------------------------------------------------------------
int vpMergeTracksDialog::itemTrackId(int index)
{
  QTE_D(vpMergeTracksDialog);

  return d->UI.trackList->topLevelItem(index)->data(0, Qt::DisplayRole).toInt();
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::validate()
{
  QTE_D(vpMergeTracksDialog);

  int numTracks = d->UI.trackList->topLevelItemCount();

  bool canReplaceMerged = true;

  // See if any of the tracks are referenced in the event model. If so, don't
  // allow a destructive merge.
  if (canReplaceMerged)
    {
    d->UI.replaceMergedTracks->setEnabled(true);
    d->UI.replaceMergedTracks->setToolTip(QString());
    }
  else
    {
    d->UI.replaceMergedTracks->setEnabled(false);
    d->UI.replaceMergedTracks->setChecked(false);
    d->UI.replaceMergedTracks->setToolTip("One or more of the tracks to merge "
                                          "is used by an event");
    }

  bool valid = numTracks > 1 && !d->UI.trackId->text().isEmpty();
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
  d->UI.status->setVisible(false);
}

//-----------------------------------------------------------------------------
void vpMergeTracksDialog::showAdvancedOptions()
{
  QTE_D(vpMergeTracksDialog);

  QDialog options;
  options.setWindowTitle("Advanced Options");

  QCheckBox* extendEvents = new QCheckBox("Extend events");
  QCheckBox* mergeEvents = new QCheckBox("Merge events");
  QSpinBox* maxMergeGap = new QSpinBox;
  QLabel* maxMergeGapLabel = new QLabel("Maximum gap (frames)");

  extendEvents->setToolTip("Extend events that start or end on the same frame "
                           "as one of the merged tracks.");
  mergeEvents->setToolTip("Merge events that lie on the same track after "
                          "tracks are merged.");
  maxMergeGapLabel->setToolTip("Maximum distance between mergeable events.");

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                     QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), &options, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), &options, SLOT(reject()));

  extendEvents->setChecked(d->AutoExtendEvents);
  mergeEvents->setChecked(d->AutoMergeEvents);
  maxMergeGap->setValue(d->MaxMergeGap);
  maxMergeGap->setMinimum(0);

  QGridLayout* layout = new QGridLayout;
  layout->addWidget(extendEvents, 0, 0);
  layout->addWidget(mergeEvents, 1, 0);
  layout->addWidget(maxMergeGapLabel, 1, 1);
  layout->addWidget(maxMergeGap, 1, 2);
  layout->addWidget(buttonBox, 2, 0, 1, 3);

  options.setLayout(layout);

  if (options.exec() == QDialog::Accepted)
    {
    QSettings settings;
    settings.beginGroup("Annotation");
    settings.setValue(ExtendEventsKey, extendEvents->isChecked());
    settings.setValue(MergeEventsKey, mergeEvents->isChecked());
    settings.setValue(MergeGapKey, maxMergeGap->value());
    this->updateOptionSettings();
    }
}

//-----------------------------------------------------------------------------
bool vpMergeTracksDialog::fixTrackReferences(vtkVgEvent* event, int trackId,
                                             int newTrackId)
{
  QTE_D(vpMergeTracksDialog);

  bool changed = false;
  for (unsigned int i = 0; i < event->GetNumberOfTracks(); ++i)
    {
    if (event->GetTrack(i)->GetId() == trackId)
      {
      //qDebug() << "changing track" << i << "in event" << event->GetId()
      //         << "from track id" << trackId
      //         << "to" << newTrackId;

      event->ReplaceTrack(i, newTrackId);
      event->SetTrackPtr(i, d->TrackModel->GetTrack(newTrackId));
      changed = true;
      }
    }

  return changed;
}

//-----------------------------------------------------------------------------
bool vpMergeTracksDialog::extendEventTrack(vtkVgEvent* event,
                                           int trackId,
                                           const vtkVgTimeStamp& prevTrackStart,
                                           const vtkVgTimeStamp& prevTrackEnd,
                                           const vtkVgTimeStamp& extendedStart,
                                           const vtkVgTimeStamp& extendedEnd)
{
  // Extending one track of a multi-track event is probably not a valid change
  // without knowing the meaning of the individual tracks in the event.
  if (event->GetNumberOfTracks() != 1)
    {
    return false;
    }

  vtkVgEventTrackInfoBase* eti = event->GetTrackInfo(0);
  if (eti->TrackId != trackId)
    {
    return false;
    }

  bool changed = false;

  // Extend single track events which started at the previous start of the
  // track or ended at the end of the previous end.
  if (extendedStart < prevTrackStart && eti->StartFrame == prevTrackStart)
    {
    qDebug() << "changing start frame of track 0 in event" << event->GetId()
             << "from" << eti->StartFrame.GetFrameNumber()
             << "to" << extendedStart.GetFrameNumber();

    event->SetTrackStartFrame(0, extendedStart);
    changed = true;
    }
  if (extendedEnd > prevTrackEnd && eti->EndFrame == prevTrackEnd)
    {
    qDebug() << "changing end frame of track 0 in event" << event->GetId()
             << "from" << eti->EndFrame.GetFrameNumber()
             << "to" << extendedEnd.GetFrameNumber();

    event->SetTrackEndFrame(0, extendedEnd);
    changed = true;
    }

  return changed;
}

//-----------------------------------------------------------------------------
bool vpMergeTracksDialog::mergeEvents(std::vector<vtkVgEvent*>& events,
                                      bool checkOnly)
{
  QTE_D(vpMergeTracksDialog);

  if (events.empty())
    {
    return false;
    }

  // Sort by start time
  std::sort(events.begin(), events.end(), EventStartsBefore());

  // Note: This algorithm will merge two events on the same track if they are
  // close enough, even if they were already 'mergeable' before the tracks were
  // merged. Not merging in this case would require keeping track of whether the
  // events were already on the same track before the track merge occurred.
  bool changed = false;
  for (size_t ia = 0; ia < events.size() - 1; ++ia)
    {
    vtkVgEvent* eventA = events[ia];
    if (eventA->GetNumberOfTracks() != 1)
      {
      continue;
      }

    int aType = eventA->GetActiveClassifierType();

    for (size_t ib = ia + 1; ib < events.size();)
      {
      vtkVgEvent* eventB = events[ib];
      int aEndFrame = static_cast<int>(eventA->GetEndFrame().GetFrameNumber());
      int bStartFrame = static_cast<int>(eventB->GetStartFrame().GetFrameNumber());

      if (bStartFrame > aEndFrame + d->MaxMergeGap)
        {
        // no later events will be mergeable with A, so set A to next event
        break;
        }

      if (eventB->GetNumberOfTracks() == 1 &&
          eventB->GetTrack(0) == eventA->GetTrack(0) &&
          eventB->GetActiveClassifierType() == aType &&
          bStartFrame - aEndFrame <= d->MaxMergeGap)
        {
        if (checkOnly)
          {
          return true;
          }

        qDebug() << "merging event" << eventB->GetId() << "into event"
                 << eventA->GetId();

        // merge these two events and remove the later occuring one
        eventA->Merge(eventB);
        d->ViewCore->deleteEvent(eventB->GetId(), d->Session);
        events.erase(events.begin() + ib);
        changed = true;
        }
      else
        {
        ++ib;
        }
      }
    }

  return changed;
}
