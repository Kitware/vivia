/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVideoArchive.h"

#include <QUrl>

#include <qtUtil.h>

#include <vgKwaFrameMetadata.h>
#include <vgKwaVideoClip.h>

#include <vtkVgAdapt.h>
#include <vgVtkVideoFrame.h>

#include <vtkVgAdaptImage.h>

#include "vsAdapt.h"
#include "vsUtilDebug.h"
#include "vsVideoHelper.h"

QTE_IMPLEMENT_D_FUNC(vsVideoArchive)

namespace // anonymous
{

//-----------------------------------------------------------------------------
vtkVgVideoFrameMetaData adaptMetadata(const vgKwaFrameMetadata& vgvMetadata)
{
  vtkVgVideoFrameMetaData vvvMetadata;

  vvvMetadata.Time = vgvMetadata.timestamp();
  vvvMetadata.Gsd = vgvMetadata.gsd();

  vvvMetadata.HomographyReferenceFrame =
    vgvMetadata.homographyReferenceFrameNumber();

  const auto& hm = vgvMetadata.homography();
  vvvMetadata.Homography->Identity();
  vvvMetadata.Homography->SetElement(0, 0, hm(0, 0));
  vvvMetadata.Homography->SetElement(0, 1, hm(0, 1));
  vvvMetadata.Homography->SetElement(0, 3, hm(0, 2));
  vvvMetadata.Homography->SetElement(1, 0, hm(1, 0));
  vvvMetadata.Homography->SetElement(1, 1, hm(1, 1));
  vvvMetadata.Homography->SetElement(1, 3, hm(1, 2));
  vvvMetadata.Homography->SetElement(3, 0, hm(2, 0));
  vvvMetadata.Homography->SetElement(3, 1, hm(2, 1));
  vvvMetadata.Homography->SetElement(3, 3, hm(2, 2));

  const vgKwaWorldBox corners = vgvMetadata.worldCornerPoints();
  vvvMetadata.WorldLocation.GCS = corners.GCS;
  vvvMetadata.WorldLocation.UpperLeft.Latitude   = corners.UpperLeft.Latitude;
  vvvMetadata.WorldLocation.UpperLeft.Longitude  = corners.UpperLeft.Longitude;
  vvvMetadata.WorldLocation.UpperRight.Latitude  = corners.UpperRight.Latitude;
  vvvMetadata.WorldLocation.UpperRight.Longitude = corners.UpperRight.Longitude;
  vvvMetadata.WorldLocation.LowerRight.Latitude  = corners.LowerRight.Latitude;
  vvvMetadata.WorldLocation.LowerRight.Longitude = corners.LowerRight.Longitude;
  vvvMetadata.WorldLocation.LowerLeft.Latitude   = corners.LowerLeft.Latitude;
  vvvMetadata.WorldLocation.LowerLeft.Longitude  = corners.LowerLeft.Longitude;

  vvvMetadata.SetWidthAndHeight(vgvMetadata.imageSize().width(),
                                vgvMetadata.imageSize().height());
  return vvvMetadata;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsVideoArchivePrivate
{
public:
  vsVideoArchivePrivate(const QUrl& clipUri);

  vtkVgTimeStamp timestamp(vxl_int_64 time);

  bool Okay;
  QUrl ClipUri;
  vgKwaVideoClip Clip;
  vsVideoHelper Helper;
};

//-----------------------------------------------------------------------------
vsVideoArchivePrivate::vsVideoArchivePrivate(const QUrl& clipUri) :
  Okay(false), ClipUri(clipUri), Clip(clipUri)
{
  if (this->Clip.frameCount())
    {
    this->Okay = true;
    qtDebug(vsdVideoArchive) << "loaded clip from" << this->ClipUri;
    }
}

//-----------------------------------------------------------------------------
vsVideoArchive::vsVideoArchive(const QUrl& archiveUri) :
  d_ptr(new vsVideoArchivePrivate(archiveUri))
{
}

//-----------------------------------------------------------------------------
vsVideoArchive::~vsVideoArchive()
{
}

//-----------------------------------------------------------------------------
bool vsVideoArchive::okay() const
{
  QTE_D_CONST(vsVideoArchive);
  return d->Okay;
}

//-----------------------------------------------------------------------------
QUrl vsVideoArchive::uri() const
{
  QTE_D_CONST(vsVideoArchive);
  return d->ClipUri;
}

//-----------------------------------------------------------------------------
void vsVideoArchive::initialize()
{
  QTE_D(vsVideoArchive);

  // Get homographies for the whole clip
  vgKwaVideoClip::MetadataMap md = d->Clip.metadata();
  if (md.count())
    {
    QList<vtkVgVideoFrameMetaData> metadata;
    foreach (const vgKwaFrameMetadata& mdi, md)
      {
      metadata.append(adaptMetadata(mdi));
      }

    // Notify user of available time range and metadata
    emit this->frameRangeAvailable(metadata.first().Time,
                                   metadata.last().Time);
    emit this->metadataAvailable(metadata);
    }
  else
    {
    // Notify user of available time range
    emit this->frameRangeAvailable(d->Clip.firstTime(), d->Clip.lastTime());
    }
}

//-----------------------------------------------------------------------------
void vsVideoArchive::requestFrame(vgVideoSeekRequest request)
{
  QTE_D(vsVideoArchive);

  // Seek to the appropriate frame
  vgVideoFramePtr frame =
    d->Helper.updateFrame(d->Clip, request);

  if (frame.isValid())
    {
    // Get metadata
    const vgKwaFrameMetadata metadata = d->Clip.metadataAt(frame.time());

    // Extract the pixels in VTK-usable format
    vgVtkVideoFramePtr rframe(new vtkVgVideoFrame(vtkVgAdapt(*frame)));

    // Build the metadata and hand the frame to the caller
    rframe->MetaData = adaptMetadata(metadata);
    request.sendReply(rframe);
    }
}

//-----------------------------------------------------------------------------
void vsVideoArchive::clearLastRequest(vgVideoSourceRequestor* requestor)
{
  QTE_D(vsVideoArchive);
  d->Helper.clearLastRequest(requestor);
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vsVideoArchive::findTime(
  unsigned int frameNumber, vg::SeekMode roundMode)
{
  QTE_D(vsVideoArchive);
  return vsVideoHelper::findTime(d->Clip, frameNumber, roundMode);
}
