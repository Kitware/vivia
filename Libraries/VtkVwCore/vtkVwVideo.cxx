/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVwVideo.h"

#include <QBuffer>
#include <QImage>
#include <QUrl>

#define JSON_INT_TYPE long long
#include <json.h>

#include <vgKwaFrameMetadata.h>
#include <vgKwaVideoClip.h>

#include "jsUtil.h"

//BEGIN JS conversion helpers

namespace // anonymous
{

//-----------------------------------------------------------------------------
qtJson::Value js(const vgKwaWorldBox& corners)
{
  if (corners.GCS == -1)
    {
    return qtJson::Value();
    }

  qtJson::Object object;

  object.insert("gcs", corners.GCS);
  object.insert("ll", vvJson::serialize(corners.LowerLeft));
  object.insert("lr", vvJson::serialize(corners.LowerRight));
  object.insert("ul", vvJson::serialize(corners.UpperLeft));
  object.insert("ur", vvJson::serialize(corners.UpperRight));

  return object;
}

//-----------------------------------------------------------------------------
QByteArray recode(const vgImage& pixels)
{
  const int ni = pixels.iCount();
  const int nj = pixels.jCount();
  const int np = pixels.planeCount();

  const ptrdiff_t si = pixels.iStep();
  const ptrdiff_t sj = pixels.jStep();
  const ptrdiff_t sp = pixels.planeStep();

  const char* const pixelData =
    reinterpret_cast<const char*>(pixels.constData());
  const int pixelDataSize = ni * nj * np;

  // Test if recoding is needed
  if (sp == 1 && si == np && sj == (ni * np))
    {
    return QByteArray(pixelData, pixelDataSize);
    }

  QByteArray result;
  result.resize(pixelDataSize);
  for (int j = 0; j < nj; ++j)
    {
    const ptrdiff_t oj = j * sj;
    const int xj = j * ni * np;
    for (int i = 0; i < ni; ++i)
      {
      const ptrdiff_t oi = i * si;
      const int xi = i * np;
      for (int p = 0; p < np; ++p)
        {
        const ptrdiff_t op = p * sp;
        result[xi + xj + p] = *(pixelData + oi + oj + op);
        }
      }
    }

  return result;
}

//-----------------------------------------------------------------------------
vgTimeStamp decodeTimeStamp(const char* jsData)
{
  vgTimeStamp result;

  const JSONNode jsObject = libjson::parse(jsData);
  const JSONNode::const_iterator end = jsObject.end();
  for (JSONNode::const_iterator iter = jsObject.begin(); iter != end; ++iter)
    {
    if (iter->name() == "time")
      {
      result.Time = iter->as_int();
      }
    else if (iter->name() == "frame")
      {
      result.FrameNumber = static_cast<unsigned int>(iter->as_int());
      }
    }

  return result;
}

} // namespace <anonymous>

//END JS conversion helpers

///////////////////////////////////////////////////////////////////////////////

//BEGIN vtkVwVideo

//-----------------------------------------------------------------------------
class vtkVwVideo::vtkInternal
{
public:
  vtkInternal(const QUrl& uri) : Clip(uri) {}

  vgKwaVideoClip Clip;
};

//-----------------------------------------------------------------------------
vtkVwVideo::vtkVwVideo() : Internal(0)
{
}

//-----------------------------------------------------------------------------
vtkVwVideo::~vtkVwVideo()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
vtkVwVideo* vtkVwVideo::New()
{
  return new vtkVwVideo();
}

//-----------------------------------------------------------------------------
void vtkVwVideo::PrintSelf(ostream& os, vtkIndent indent)
{
  // TODO
  vtkObject::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
bool vtkVwVideo::Open(const char* uri)
{
  delete this->Internal;
  this->Internal = new vtkInternal(QUrl(QString::fromLocal8Bit(uri)));
  return this->Internal->Clip.frameCount() > 0;
}

//-----------------------------------------------------------------------------
std::string vtkVwVideo::GetFirstTime() const
{
  qtJson::Value result;
  if (this->Internal)
    {
    result = vvJson::serialize(this->Internal->Clip.firstTime());
    }
  return jsUtil::encode(result);
}

//-----------------------------------------------------------------------------
std::string vtkVwVideo::GetLastTime() const
{
  qtJson::Value result;
  if (this->Internal)
    {
    result = vvJson::serialize(this->Internal->Clip.lastTime());
    }
  return jsUtil::encode(result);
}

//-----------------------------------------------------------------------------
int vtkVwVideo::GetFrameCount() const
{
  if (this->Internal)
    {
    return this->Internal->Clip.frameCount();
    }
  return 0;
}

//-----------------------------------------------------------------------------
std::string vtkVwVideo::GetFrame(const char* jsReferenceTime,
                                 int direction) const
{
  qtJson::Value result;
  if (this->Internal)
    {
    // Get requested frame
    const vgVideoFramePtr frame =
      this->Internal->Clip.frameAt(decodeTimeStamp(jsReferenceTime),
                                   static_cast<vg::SeekMode>(direction));

    // If frame is valid, construct result object
    if (frame.isValid())
      {
      qtJson::Object jsFrame;
      jsFrame.insert("t", vvJson::serialize(frame.time()));

      // Get pixel data
      const QImage pixels = frame.image().toQImage();
      QByteArray pixelData;
      QBuffer buffer(&pixelData);
      buffer.open(QIODevice::WriteOnly);
      pixels.save(&buffer, "JPG");

      // Add pixel data to the JSON object
      jsFrame.insert("w", pixels.width());
      jsFrame.insert("h", pixels.height());
      jsFrame.insert("format", "image/jpeg");
      jsFrame.insert("d", pixelData.toBase64().prepend('\"').append('\"'));

      // Get metadata
      const vgKwaFrameMetadata md =
        this->Internal->Clip.metadataAt(frame.time(), vg::SeekExact);

      // Add metadata to JSON object
      jsFrame.insert("gsd", md.gsd());
      jsFrame.insert("corners", js(md.worldCornerPoints()));
      // TODO homography matrix and reference time

      result = jsFrame;
      }
    }

  // Return encoded frame
  return jsUtil::encode(result);
}

//END vtkVwVideo
