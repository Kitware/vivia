/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgKwaFrameMetadata.h"

#include <vil/io/vil_io_image_view.h>
#include <vnl/io/vnl_io_matrix_fixed.h>
#include <vnl/io/vnl_io_vector_fixed.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vsl/vsl_binary_io.h>
#include <vsl/vsl_vector_io.h>

#include <vector>

const int vgKwaFrameMetadata::SupportedDataVersion = 3;

//-----------------------------------------------------------------------------
class vgKwaFrameMetadataData : public QSharedData
{
public:
  vgTimeStamp timestamp;
  vgMatrix3d homography;
  uint homographyReferenceFrameNumber;

  vgKwaWorldBox worldCornerPoints;

  double gsd;
  QSize imageSize;
};

QTE_IMPLEMENT_D_FUNC_SHARED(vgKwaFrameMetadata)

//-----------------------------------------------------------------------------
vgKwaFrameMetadata::vgKwaFrameMetadata() : d_ptr(new vgKwaFrameMetadataData)
{
  QTE_D_MUTABLE(vgKwaFrameMetadata);
  d->homographyReferenceFrameNumber = -1;
  d->gsd = -1.0;
}

//-----------------------------------------------------------------------------
vgKwaFrameMetadata::vgKwaFrameMetadata(
  vsl_b_istream& stream, int version, bool isMeta) :
  d_ptr(new vgKwaFrameMetadataData)
{
  Q_ASSERT(version > 0 && version <= vgKwaFrameMetadata::SupportedDataVersion);

  QTE_D_MUTABLE(vgKwaFrameMetadata);

  stream.clear_serialisation_records();

  vxl_int_64 time;
  vnl_matrix_fixed<double, 3, 3> homography;
  std::vector<vnl_vector_fixed<double, 2> > corners;

  vsl_b_read(stream, time);
  if (!isMeta)
    {
    if (version < 3)
      {
      vil_image_view<vxl_byte> image;
      vsl_b_read(stream, image);
      }
    else
      {
      char imageFormat;
      std::vector<char> imageData;
      vsl_b_read(stream, imageFormat);
      vsl_b_read(stream, imageData);
      }
    }
  vsl_b_read(stream, homography);
  vsl_b_read(stream, corners);

  d->timestamp.Time = time;

  d->homography = vgMatrix3d{homography.data_block()};

  double max = qMax(qMax(qMax(corners[0][0], corners[0][1]),
                         qMax(corners[1][0], corners[1][1])),
                    qMax(qMax(corners[2][0], corners[2][1]),
                         qMax(corners[3][0], corners[3][1])));

  if (max < 400)
    {
    d->worldCornerPoints.GCS = 4326;
    d->worldCornerPoints.UpperLeft.Latitude   = corners[0][0];
    d->worldCornerPoints.UpperLeft.Longitude  = corners[0][1];
    d->worldCornerPoints.UpperRight.Latitude  = corners[1][0];
    d->worldCornerPoints.UpperRight.Longitude = corners[1][1];
    d->worldCornerPoints.LowerLeft.Latitude   = corners[2][0];
    d->worldCornerPoints.LowerLeft.Longitude  = corners[2][1];
    d->worldCornerPoints.LowerRight.Latitude  = corners[3][0];
    d->worldCornerPoints.LowerRight.Longitude = corners[3][1];
    }
  else
    {
    d->worldCornerPoints.GCS = -1;
    }

  if (version > 1)
    {
    vxl_int_64 frameNumber;
    vxl_int_64 homographyReferenceFrameNumber;
    vxl_int_64 imageWidth;
    vxl_int_64 imageHeight;

    vsl_b_read(stream, d->gsd);
    vsl_b_read(stream, frameNumber);
    vsl_b_read(stream, homographyReferenceFrameNumber);
    vsl_b_read(stream, imageWidth);
    vsl_b_read(stream, imageHeight);

    d->timestamp.FrameNumber = static_cast<unsigned int>(frameNumber);
    d->homographyReferenceFrameNumber = homographyReferenceFrameNumber;
    d->imageSize.setWidth(static_cast<int>(imageWidth));
    d->imageSize.setHeight(static_cast<int>(imageHeight));
    }
}

//-----------------------------------------------------------------------------
vgKwaFrameMetadata::~vgKwaFrameMetadata()
{
}

//-----------------------------------------------------------------------------
vgKwaFrameMetadata::vgKwaFrameMetadata(const vgKwaFrameMetadata& other) :
  d_ptr(other.d_ptr)
{
}

//-----------------------------------------------------------------------------
vgKwaFrameMetadata& vgKwaFrameMetadata::operator=(
  const vgKwaFrameMetadata& other)
{
  this->d_ptr = other.d_ptr;
  return *this;
}

//-----------------------------------------------------------------------------
vgTimeStamp vgKwaFrameMetadata::timestamp() const
{
  QTE_D_SHARED(vgKwaFrameMetadata);
  return d->timestamp;
}

//-----------------------------------------------------------------------------
vgMatrix3d vgKwaFrameMetadata::homography() const
{
  QTE_D_SHARED(vgKwaFrameMetadata);
  return d->homography;
}

//-----------------------------------------------------------------------------
uint vgKwaFrameMetadata::homographyReferenceFrameNumber() const
{
  QTE_D_SHARED(vgKwaFrameMetadata);
  return d->homographyReferenceFrameNumber;
}

//-----------------------------------------------------------------------------
vgKwaWorldBox vgKwaFrameMetadata::worldCornerPoints() const
{
  QTE_D_SHARED(vgKwaFrameMetadata);
  return d->worldCornerPoints;
}

//-----------------------------------------------------------------------------
double vgKwaFrameMetadata::gsd() const
{
  QTE_D_SHARED(vgKwaFrameMetadata);
  return d->gsd;
}

//-----------------------------------------------------------------------------
QSize vgKwaFrameMetadata::imageSize() const
{
  QTE_D_SHARED(vgKwaFrameMetadata);
  return d->imageSize;
}
