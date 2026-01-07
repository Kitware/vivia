// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqVideoQueryDialog.h"

#include <vvVideoQueryDialogPrivate.h>

#include <QMessageBox>

// Qt Extensions includes
#include <qtStatusManager.h>
#include <qtStlUtil.h>

// Qt includes
#include <QList>

// visgui includes
#include <vgFileDialog.h>
#include <vgUnixTime.h>

// VV includes
#include <vvMakeId.h>
#include <vvQuery.h>
#include <vvQueryService.h>
#include <vvQuerySession.h>

// viqui includes
#include "vqDebug.h"
#include "vqQueryVideoPlayer.h"
#include "vqSettings.h"

//-----------------------------------------------------------------------------
class vqVideoQueryDialogPrivate : public vvVideoQueryDialogPrivate
{
public:
  vqVideoQueryDialogPrivate(vvVideoQueryDialog* q) :
    vvVideoQueryDialogPrivate(q, new vqQueryVideoPlayer),
    StartTime(-1),
    EndTime(-1)
    {}

  void disableVideoSelectionAndProcessing()
    {
    this->UI.chooseVideo->setEnabled(false);
    this->UI.videoLocation->setEnabled(false);
    this->UI.reprocessVideo->setEnabled(false);
    }

  QUrl ExemplarUri;
  vvQueryFormulationType FormulationType;

  double StartTime;
  double EndTime;
};

QTE_IMPLEMENT_D_FUNC(vqVideoQueryDialog)

///////////////////////////////////////////////////////////////////////////////

//BEGIN vqVideoQueryDialog public interface

//-----------------------------------------------------------------------------
vqVideoQueryDialog::vqVideoQueryDialog(bool useAdvancedUi,
                                       QWidget* parent, Qt::WindowFlags flags)
  : vvVideoQueryDialog(new vqVideoQueryDialogPrivate(this),
                       useAdvancedUi, parent, flags)
{
}

//-----------------------------------------------------------------------------
vqVideoQueryDialog::~vqVideoQueryDialog()
{
}

//-----------------------------------------------------------------------------
std::string vqVideoQueryDialog::exemplarUri() const
{
  QTE_D_CONST(vqVideoQueryDialog);
  return stdString(d->ExemplarUri);
}

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::setExemplarUri(
  const std::string& uri, vvQueryFormulationType formulationType)
{
  QTE_D(vqVideoQueryDialog);

  d->FormulationType = formulationType;

  if (!uri.empty())
    {
    const QUrl server = vqSettings().queryServerUri();
    QScopedPointer<vvQuerySession> session(
      vvQueryService::createSession(server));
    if (!session)
      {
      QMessageBox::warning(
        this, "Failed to create session",
        "No handler available for query server URI scheme " +
        server.scheme() + ". Please check your configuration.");
      return;
      }

    QUrl qUri = qtUrl(uri);
    d->ExemplarUri =
      session->fixupFormulationSourceUri(qUri, formulationType);
    }
  else
    {
    d->ExemplarUri.clear();
    }
}

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::setTimeRange(
  double startTime, double endTime, double initialTime)
{
  QTE_D(vqVideoQueryDialog);

  d->StartTime = startTime;
  d->EndTime = endTime;
  d->InitialTime = vgTimeStamp::fromTime(initialTime);
}

//-----------------------------------------------------------------------------
int vqVideoQueryDialog::exec()
{
  QTE_D(vqVideoQueryDialog);

  if (d->ExemplarUri.isEmpty())
    {
    // If we don't have a video yet, ask for one to process
    this->chooseQueryVideo();

    // Abort if user did not pick a video
    if (d->ExemplarUri.isEmpty())
      {
      this->reject();
      return QDialog::Rejected;
      }
    }

  return vvVideoQueryDialog::exec();
}

//END vqVideoQueryDialog public interface

///////////////////////////////////////////////////////////////////////////////

//BEGIN vqVideoQueryDialog query formulation handling

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::chooseQueryVideo()
{
  QTE_D(vqVideoQueryDialog);

  const QUrl server = vqSettings().queryServerUri();
  QScopedPointer<vvQuerySession> session(
    vvQueryService::createSession(server));
  if (!session)
    {
    QMessageBox::warning(
      this, "Failed to create session",
      "No handler available for query server URI scheme " +
      server.scheme() + ". Please check your configuration.");
    return;
    }

  const QUrl uri = session->getFormulationSourceUri(
                     d->ExemplarUri, d->FormulationType, this);
  if (!uri.isEmpty())
    {
    d->ExemplarUri = uri;
    this->processQueryVideo(false);
    }
}

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::reprocessQueryVideo()
{
  this->processQueryVideo(true);
}

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::setSelectedDescriptors(
  const std::vector<vvDescriptor>& descriptors)
{
  QTE_D(vqVideoQueryDialog);

  vvVideoQueryDialog::setSelectedDescriptors(descriptors);

  if (!d->ExemplarUri.isEmpty())
    {
    switch (d->FormulationType)
      {
      case vvQueryFormulation::FromDatabase:
        // \TODO not only do we need to process the query, but there is a lot
        //       of stuff done by processQueryVideo that probably needs to be
        //       done as well
        break;
      default:
        this->processQueryVideo(false);
        break;
      }
    }
}

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::setQueryTracksAndDescriptors(
  QList<vvDescriptor> descriptors, QList<vvTrack> tracks)
{
  QTE_D(vqVideoQueryDialog);

  // Load video
  // \TODO: If this is a database query, find the video in the vgKwaArchive
  //        (might need to build a private instance?), NOT relative to the QF
  //        KWA location
  vqQueryVideoPlayer* player = static_cast<vqQueryVideoPlayer*>(d->Player);
  player->setTimeRange(d->StartTime, d->EndTime);
  if (!player->setVideoUri(d->ExemplarUri))
    {
    // Treat missing archive as QF failure
    d->StatusManager.setStatusText(
      d->StatusSource, "Failed to load processed video data");
    this->clearQueryDescriptors();
    return;
    }

  if (d->FormulationType == vvQueryFormulation::FromDatabase)
    {
    descriptors = this->filterDescriptors(descriptors);
    vvVideoQueryDialog::setQueryTracksAndDescriptors(descriptors, tracks);

    d->disableVideoSelectionAndProcessing();
    }
  else
    {
    vvVideoQueryDialog::setQueryTracksAndDescriptors(descriptors, tracks);
    }
}

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::clearQueryDescriptors()
{
  QTE_D(vqVideoQueryDialog);

  vqQueryVideoPlayer* player = static_cast<vqQueryVideoPlayer*>(d->Player);
  player->setVideoUri(QUrl());

  vvVideoQueryDialog::clearQueryDescriptors();
}

//-----------------------------------------------------------------------------
void vqVideoQueryDialog::processQueryVideo(bool bypassCache)
{
  QTE_D(vqVideoQueryDialog);

  if (!this->isVisible())
    {
    // \NOTE: We are explicitly calling show() here as this forces Qt to create
    //        and initialize render context; needed so that setting the initial
    //        view / camera will work
    this->show();
    this->hide();
    }

  d->StatusManager.setStatusText(d->StatusSource);

  // Clear old results
  d->SelectedDescriptors.clear();
  d->UI.trackDescriptors->clear();
  d->UI.selectedDescriptors->clear();
  d->UI.regionDescriptors->clear();

  // Disable UI while processing video
  this->setSelectionControlsEnabled(false);

  // Disallow accept with nothing selected
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  // Construct the formulation query
  vvProcessingRequest request;
  request.QueryId = vvMakeId("VIQUI-QF");
  request.VideoUri = stdString(d->ExemplarUri);
  d->UI.videoLocation->setText(d->ExemplarUri.toString());

  // Request results from the core
  // TODO: We should be using a private parser for this, once we trust all of
  //       our back-ends to tolerate multiple concurrent users, and get rid of
  //       needing to know about vqCore at all
  emit this->queryFormulationRequested(request, bypassCache,
                                       &d->StatusManager);
}

//-----------------------------------------------------------------------------
QList<vvDescriptor> vqVideoQueryDialog::filterDescriptors(
  QList<vvDescriptor> descriptors)
{
  QTE_D(vqVideoQueryDialog);

  QList<vvDescriptor> filteredDescriptors;

  foreach (vvDescriptor descriptor, descriptors)
    {
    if (descriptor.ModuleName == "MergedDescriptors")
      {
      filteredDescriptors.append(descriptor);
      continue;
      }

    vgTimeStamp startTime = descriptor.Region.begin()->TimeStamp;
    vgTimeStamp endTime = descriptor.Region.rbegin()->TimeStamp;

    if ((startTime.Time >= d->StartTime)
        && (endTime.Time <= d->EndTime))
      {
      filteredDescriptors.append(descriptor);
      }
    }

  return filteredDescriptors;
}

//END vqVideoQueryDialog query formulation handling
