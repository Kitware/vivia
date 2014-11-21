/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgKwaFrameMetadata_h
#define __vgKwaFrameMetadata_h

#include <QMatrix3x3>
#include <QSharedDataPointer>
#include <QSize>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vgGeoTypes.h>
#include <vgTimeStamp.h>

class vsl_b_istream;

class vgKwaFrameMetadataData;

//-----------------------------------------------------------------------------
struct vgKwaWorldBox : vgGeoSystem
{
  vgGeoRawCoordinate UpperLeft;
  vgGeoRawCoordinate UpperRight;
  vgGeoRawCoordinate LowerLeft;
  vgGeoRawCoordinate LowerRight;
};

//-----------------------------------------------------------------------------
class VG_VIDEO_EXPORT vgKwaFrameMetadata
{
public:
  vgKwaFrameMetadata();
  virtual ~vgKwaFrameMetadata();

  vgKwaFrameMetadata(const vgKwaFrameMetadata&);
  vgKwaFrameMetadata& operator=(const vgKwaFrameMetadata&);

  vgTimeStamp timestamp() const;
  QMatrix3x3 homography() const;
  uint homographyReferenceFrameNumber() const;

  vgKwaWorldBox worldCornerPoints() const;

  double gsd() const;
  QSize imageSize() const;

  static const int SupportedDataVersion;

protected:
  QTE_DECLARE_SHARED_PTR(vgKwaFrameMetadata)

  friend class vgKwaVideoClip;
  friend class vgKwaVideoClipPrivate;

  vgKwaFrameMetadata(vsl_b_istream& stream, int version, bool isMeta);

private:
  QTE_DECLARE_SHARED(vgKwaFrameMetadata)
};

#endif
