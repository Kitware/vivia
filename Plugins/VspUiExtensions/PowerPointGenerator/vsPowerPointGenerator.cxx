// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsPowerPointGenerator.h"

#include <vsEventUserInfo.h>

#include <vsVideoSource.h>

#include <vvPowerPointEventSlideGenerator.h>
#include <vvPowerPointExtensionInterface.h>
#include <vvPowerPointTrackSlideGenerator.h>
#include <vvPowerPointWriter.h>

#include <vgVideoRequestor.h>

#include <vgPluginLoader.h>

#include <vtkVgEvent.h>
#include <vtkVgTrack.h>

#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>

namespace // anonymous
{

//-----------------------------------------------------------------------------
static bool trackEarlierThan(const vtkVgTrack* a,
                             const vtkVgTrack* b)
{
  return a->GetStartFrame() < b->GetStartFrame();
}

//-----------------------------------------------------------------------------
static bool eventEarlierThan(const vsEventUserInfo& a,
                             const vsEventUserInfo& b)
{
  return a.Event->GetStartFrame() < b.Event->GetStartFrame();
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsPowerPointGeneratorPrivate
{
public:
  vsPowerPointGeneratorPrivate(vsVideoSource* source)
    : VideoRequestor(source) {}

  QString OutputPath;
  QString TemplateFile;
  QList<vtkVgTrack*> Tracks;
  QList<vsEventUserInfo> Events;
  vtkVgEventTypeRegistry* Registry;

  vgVideoRequestor VideoRequestor;
};

QTE_IMPLEMENT_D_FUNC(vsPowerPointGenerator)

//-----------------------------------------------------------------------------
vsPowerPointGenerator::vsPowerPointGenerator(QList<vsEventUserInfo> events,
                                             QList<vtkVgTrack*> tracks,
                                             vtkVgEventTypeRegistry* registry,
                                             vsVideoSource* videoSource)
  : d_ptr(new vsPowerPointGeneratorPrivate(videoSource))
{
  Q_ASSERT(videoSource);

  QTE_D(vsPowerPointGenerator);
  d->Tracks = tracks;
  d->Events = events;
  d->Registry = registry;
}

//-----------------------------------------------------------------------------
vsPowerPointGenerator::~vsPowerPointGenerator()
{
}

//-----------------------------------------------------------------------------
void vsPowerPointGenerator::setOutputPath(const QString& path)
{
  QTE_D(vsPowerPointGenerator);

  d->OutputPath = path;
}

//-----------------------------------------------------------------------------
void vsPowerPointGenerator::setTemplateFile(const QString& templateFile)
{
  QTE_D(vsPowerPointGenerator);

  d->TemplateFile = templateFile;
}

//-----------------------------------------------------------------------------
void vsPowerPointGenerator::generatePowerPoint(bool generateVideo)
{
  QTE_D(vsPowerPointGenerator);

  Q_ASSERT(!d->OutputPath.isEmpty());

  // Sort by track start time
  qSort(d->Tracks.begin(), d->Tracks.end(), trackEarlierThan);

  // Sort by event start time
  qSort(d->Events.begin(), d->Events.end(), eventEarlierThan);

  // List of items to process (manually delete the objects in the list when
  // done with them)
  vvPowerPointInput pptInput;
  vvPowerPointItemCollection itemCollection;

  // Tracks
  pptInput.tracks = d->Tracks;
  foreach (vtkVgTrack* track, d->Tracks)
    {
    if (track->IsStarred())
      {
      vvPowerPointTrackSlideGenerator* trackSlide =
        new vvPowerPointTrackSlideGenerator(track);
      itemCollection.items.append(trackSlide);
      itemCollection.itemMap.insert(track, trackSlide);
      }
    }

  // Events
  foreach (vsEventUserInfo info, d->Events)
    {
    pptInput.events.append(info.Event);
    if (info.Event->IsStarred())
      {
      vvPowerPointEventSlideGenerator* eventSlide =
        new vvPowerPointEventSlideGenerator(info.Event, d->Registry);
      itemCollection.items.append(eventSlide);
      itemCollection.itemMap.insert(info.Event, eventSlide);
      }
    }

  // Cycle through any PPT extensions
  foreach(vvPowerPointExtensionInterface* plugin,
          vgPluginLoader::pluginInterfaces<vvPowerPointExtensionInterface>())
    {
    plugin->updateItems(pptInput, itemCollection);
    }

  // Count the total number of work items
  int total = 0;
  foreach(vvPowerPointSlideGenerator* item, itemCollection.items)
    {
    total += item->outputFrameCount(generateVideo);
    }

  // Pop up progress bar
  QProgressDialog progress("Generating PowerPoint...", "Cancel", 0, total);
  progress.setWindowModality(Qt::ApplicationModal);
  progress.setMinimumDuration(0);
  progress.setAutoClose(false);
  progress.setAutoReset(false);
  progress.setValue(0);

  int progressCount = 0;
  int outputId = 0;

  vvPowerPointWriter writer(d->OutputPath, d->TemplateFile);
  foreach(vvPowerPointSlideGenerator* item, itemCollection.items)
    {
    if (!item->generateSlide(writer, d->VideoRequestor, ++outputId,
                             generateVideo, &progress, progressCount))
      {
      break;
      }
    }

  qDeleteAll(itemCollection.items);
}
