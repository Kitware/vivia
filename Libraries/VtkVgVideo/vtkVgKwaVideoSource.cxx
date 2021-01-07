// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgKwaVideoSource.h"

#include <QUrl>

#include <vtkObjectFactory.h>

#include <vgKwaFrameMetadata.h>
#include <vgKwaVideoClip.h>

#include <vtkVgTimeStamp.h>
#include <vtkVgVideoFrame.h>

#include "vtkVgAdaptImage.h"

namespace // anonymous
{

//-----------------------------------------------------------------------------
vtkVgVideoFrameCorner vtkVgAdapt(const vgGeoRawCoordinate& in)
{
  vtkVgVideoFrameCorner out;
  out.Latitude = in.Latitude;
  out.Longitude = in.Longitude;
  return out;
}

//-----------------------------------------------------------------------------
vtkVgVideoFrameMetaData vtkVgAdapt(const vgKwaFrameMetadata& in)
{
  vtkVgVideoFrameMetaData result;

  result.Time = in.timestamp();
  result.Gsd = in.gsd();

  result.Width = in.imageSize().width();
  result.Height = in.imageSize().height();

  result.Homography->Identity();
  result.Homography->SetElement(0, 0, in.homography()(0, 0));
  result.Homography->SetElement(0, 1, in.homography()(0, 1));
  result.Homography->SetElement(0, 3, in.homography()(0, 2));
  result.Homography->SetElement(1, 0, in.homography()(1, 0));
  result.Homography->SetElement(1, 1, in.homography()(1, 1));
  result.Homography->SetElement(1, 3, in.homography()(1, 2));
  result.Homography->SetElement(3, 0, in.homography()(2, 0));
  result.Homography->SetElement(3, 1, in.homography()(2, 1));
  result.Homography->SetElement(3, 3, in.homography()(2, 2));
  result.HomographyReferenceFrame = in.homographyReferenceFrameNumber();

  const vgKwaWorldBox& corners = in.worldCornerPoints();
  result.WorldLocation.GCS = corners.GCS;
  result.WorldLocation.UpperLeft = vtkVgAdapt(corners.UpperLeft);
  result.WorldLocation.UpperRight = vtkVgAdapt(corners.UpperRight);
  result.WorldLocation.LowerLeft = vtkVgAdapt(corners.LowerLeft);
  result.WorldLocation.LowerRight = vtkVgAdapt(corners.LowerRight);

  return result;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vtkVgKwaVideoSource::vtkInternal
{
public:
  vtkInternal(const QUrl& uri) : Clip(uri) {}

  vgKwaVideoClip Clip;
};

vtkStandardNewMacro(vtkVgKwaVideoSource);

//-----------------------------------------------------------------------------
vtkVgKwaVideoSource::vtkVgKwaVideoSource() : Internal(0)
{
}

//-----------------------------------------------------------------------------
vtkVgKwaVideoSource::~vtkVgKwaVideoSource()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgKwaVideoSource::PrintSelf(ostream& os, vtkIndent indent)
{
  // TODO
  Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
bool vtkVgKwaVideoSource::Open(const char* uri)
{
  delete this->Internal;
  this->Internal = new vtkInternal(QUrl(QString::fromLocal8Bit(uri)));
  return this->Internal->Clip.frameCount() > 0;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgKwaVideoSource::GetMinTime() const
{
  if (this->Internal)
    {
    return this->Internal->Clip.firstTime();
    }
  return vtkVgTimeStamp();
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgKwaVideoSource::GetMaxTime() const
{
  if (this->Internal)
    {
    return this->Internal->Clip.lastTime();
    }
  return vtkVgTimeStamp();
}

//-----------------------------------------------------------------------------
int vtkVgKwaVideoSource::GetFrameCount() const
{
  if (this->Internal)
    {
    return this->Internal->Clip.frameCount();
    }
  return 0;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgKwaVideoSource::ResolveSeek(
  const vtkVgTimeStamp& ts, vg::SeekMode direction) const
{
  if (this->Internal)
    {
    // Get requested frame
    const vgVideoFramePtr frame =
      this->Internal->Clip.frameAt(ts.GetRawTimeStamp(), direction);
    return frame.time();
    }
  return vtkVgTimeStamp();
}

//-----------------------------------------------------------------------------
vtkVgVideoFrame vtkVgKwaVideoSource::GetFrame(
  const vtkVgTimeStamp& ts, vg::SeekMode direction) const
{
  if (this->Internal)
    {
    // Get requested frame
    const vgVideoFramePtr frame =
      this->Internal->Clip.frameAt(ts.GetRawTimeStamp(), direction);

    // If frame is valid, construct result object
    if (frame.isValid())
      {
      vtkVgVideoFrame result(vtkVgAdapt(frame.image()));
      result.MetaData.Time = frame.time();

      // Get metadata
      const vgKwaFrameMetadata md =
        this->Internal->Clip.metadataAt(frame.time(), vg::SeekExact);
      result.MetaData = vtkVgAdapt(md);

      return result;
      }
    }

  // Return encoded frame
  return vtkVgVideoFrame();
}

//-----------------------------------------------------------------------------
vtkVgVideoFrameMetaData vtkVgKwaVideoSource::GetMetadata(
  const vtkVgTimeStamp& ts, vg::SeekMode direction) const
{
  if (this->Internal)
    {
    // Get requested frame
    const vgKwaFrameMetadata md =
      this->Internal->Clip.metadataAt(ts.GetRawTimeStamp(), direction);

    // If frame is valid, construct result object
    if (md.timestamp().IsValid())
      {
      return vtkVgAdapt(md);
      }
    }

  return vtkVgVideoFrameMetaData();
}
