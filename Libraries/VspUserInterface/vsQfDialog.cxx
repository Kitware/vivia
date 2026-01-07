// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsQfDialog.h"

#include <vvVideoQueryDialogPrivate.h>

#include <vvDescriptor.h>

#include "vsQfVideoPlayer.h"
#include "vsQfVideoSource.h"

//-----------------------------------------------------------------------------
class vsQfDialogPrivate : public vvVideoQueryDialogPrivate
{
public:
  // \note: This code could be moved somewhere else
  enum DescriptorsFilterMode
    {
    Closed = 0
    };

  vsQfDialogPrivate(vvVideoQueryDialog* q, vsQfVideoSource* source) :
    vvVideoQueryDialogPrivate(q, new vsQfVideoPlayer(source)) {}

  QList<vvDescriptor> filterDescriptors(
    vsQfVideoSource* source, QList<vvDescriptor> descriptors,
    DescriptorsFilterMode mode = Closed)
    {
    if (!source)
      {
      return descriptors;
      }

    QList<vvDescriptor> filteredDescriptors;

    double timeRange[2];
    source->GetTimeRange(timeRange);

    foreach (vvDescriptor descriptor, descriptors)
      {
      vgTimeStamp startTime = descriptor.Region.begin()->TimeStamp;
      vgTimeStamp endTime = descriptor.Region.rbegin()->TimeStamp;

      switch (mode)
        {
        case Closed:
          if ((startTime.Time >= timeRange[0])
              && (endTime.Time <= timeRange[1]))
            {
            filteredDescriptors.append(descriptor);
            }
          continue;
        default:
          filteredDescriptors.append(descriptor);
          break;
        }
      }

    return filteredDescriptors;
    }
};

QTE_IMPLEMENT_D_FUNC(vsQfDialog)

//-----------------------------------------------------------------------------
vsQfDialog::vsQfDialog(vsQfVideoSource* source,
                       QWidget* parent, Qt::WindowFlags flags)
  : vvVideoQueryDialog(new vsQfDialogPrivate(this, source),
                       true, parent, flags)
{
  QTE_D(vsQfDialog);

  d->UI.chooseVideo->setEnabled(false);
  d->UI.reprocessVideo->setEnabled(false);
  d->UI.videoLocation->setEnabled(false);
}

//-----------------------------------------------------------------------------
vsQfDialog::~vsQfDialog()
{
}

//-----------------------------------------------------------------------------
int vsQfDialog::exec()
{
  // TODO: Load video

  return vvVideoQueryDialog::exec();
}

//-----------------------------------------------------------------------------
void vsQfDialog::setQueryTracksAndDescriptors(
  QList<vvDescriptor> descriptors, QList<vvTrack> tracks)
{
  QTE_D(vsQfDialog);

  vsQfVideoPlayer* qfVideoPlayer =
    qobject_cast<vsQfVideoPlayer*>(d->Player);
  vsQfVideoSource* source = 0;
  if (qfVideoPlayer)
    {
    source = qfVideoPlayer->source();
    }

  if (source)
    {
    descriptors = d->filterDescriptors(source, descriptors);
    }
  vvVideoQueryDialog::setQueryTracksAndDescriptors(descriptors, tracks);
}

//-----------------------------------------------------------------------------
void vsQfDialog::clearQueryDescriptors()
{
  // TODO

  vvVideoQueryDialog::clearQueryDescriptors();
}
