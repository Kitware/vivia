// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpFrameMap.h"

#include "vpFileDataSource.h"
#include "vpImageSourceFactory.h"

#include <vtkVgBaseImageSource.h>
#include <vtkVgTimeStamp.h>

#include <vgTimeMap.h>

#include <qtDebug.h>
#include <qtEnumerate.h>
#include <qtGet.h>
#include <qtStlUtil.h>

#include <QAtomicInt>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>

QTE_IMPLEMENT_D_FUNC(vpFrameMap)

//-----------------------------------------------------------------------------
class vpFrameMetaData
{
public:
  vpFrameMetaData(double microseconds) : Time(microseconds), Homography(0)
    {}
  vpFrameMetaData(vtkMatrix4x4 *homography) : Time(vgTimeStamp::InvalidTime())
    {
    this->Homography = 0;
    if (homography)
      {
      this->Homography = vtkMatrix4x4::New();
      this->Homography->DeepCopy(homography);
      }
    }
  vpFrameMetaData(const vpFrameMetaData& other) : Homography(0)
    {
    this->Time = other.Time;
    if (other.Homography)
      {
      this->Homography = vtkMatrix4x4::New();
      this->Homography->DeepCopy(other.Homography);
      }
    }
  vpFrameMetaData& operator=(const vpFrameMetaData& other)
    {
    this->Time = other.Time;
    vpFrameMetaData::SetHomography(this->Homography, other.Homography);

    return *this;
    }

  ~vpFrameMetaData()
    {
    if (this->Homography)
      {
      this->Homography->UnRegister(0);
      }
    }

  static void SetHomography(vtkMatrix4x4*& destination,
                            const vtkMatrix4x4* source)
    {
    if (!source)
      {
      if (destination)
        {
        destination->Delete();
        destination = 0;
        }
      }
    else
      {
      if (!destination)
        {
        destination = vtkMatrix4x4::New();
        }
      destination->DeepCopy(source);
      }
    }

  double Time; // (microseconds)
  vtkMatrix4x4* Homography;
};

//-----------------------------------------------------------------------------
class vpFrameMapPrivate
{
public:
  vpFileDataSource* DataSource;

  QStringList FrameNames;
  vgTimeMap<int> TimeToImageMap;
  QHash<QString, vpFrameMetaData> ImageToMetaDataMap;

  QAtomicInt Progress = 0;
  volatile bool Stop = false;

  QMutex Mutex;
};

//-----------------------------------------------------------------------------
void vpFrame::set(int index, vgTimeStamp time, const vtkMatrix4x4* homography)
{
  this->Index = index;
  this->Time = time;
  vpFrameMetaData::SetHomography(this->Homography, homography);
}

//-----------------------------------------------------------------------------
vpFrameMap::vpFrameMap(vpFileDataSource* dataSource)
  : d_ptr{new vpFrameMapPrivate}
{
  QTE_D();

  d->DataSource = dataSource;
  connect(this, SIGNAL(finished()), SIGNAL(updated()));
}

//-----------------------------------------------------------------------------
vpFrameMap::~vpFrameMap()
{
  this->stop();
}

//-----------------------------------------------------------------------------
bool vpFrameMap::find(const vtkVgTimeStamp& time, vpFrame& frame,
                      vg::SeekMode seekMode)
{
  QTE_D();

  synchronized (&d->Mutex)
    {
    const auto iter = d->TimeToImageMap.find(time.GetRawTimeStamp(), seekMode);

    if (iter == d->TimeToImageMap.end())
      {
      return false;
      }

    // We need to find the entry in the image to metadata map; has to be there
    // because we build the TimeToImageMap from ImageToMetaDataMap
    const auto& frameName = d->DataSource->frameName(iter.value());
    const auto& metaData = *qtGet(d->ImageToMetaDataMap, frameName);
    frame.set(iter.value(), iter.key(), metaData.Homography);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpFrameMap::isEmpty()
{
  QTE_D();

  QMutexLocker lock{&d->Mutex};
  return d->TimeToImageMap.isEmpty();
}

//-----------------------------------------------------------------------------
bool vpFrameMap::first(vpFrame& frame)
{
  QTE_D();

  synchronized (&d->Mutex)
    {
    if (d->TimeToImageMap.isEmpty())
      {
      return false;
      }

    const auto iter = d->TimeToImageMap.begin();

    // We need to find the entry in the image to metadata map; has to be there
    // because we build the TimeToImageMap from ImageToMetaDataMap
    const auto& frameName = d->DataSource->frameName(iter.value());
    const auto& metaData = *qtGet(d->ImageToMetaDataMap, frameName);
    frame.set(iter.value(), iter.key(), metaData.Homography);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpFrameMap::last(vpFrame& frame)
{
  QTE_D();

  synchronized (&d->Mutex)
    {
    if (d->TimeToImageMap.isEmpty())
      {
      return false;
      }

    const auto iter = --d->TimeToImageMap.end();

    // We need to find the entry in the image to metadata map; has to be there
    // because we build the TimeToImageMap from ImageToMetaDataMap
    const auto& frameName = d->DataSource->frameName(iter.value());
    const auto& metaData = *qtGet(d->ImageToMetaDataMap, frameName);
    frame.set(iter.value(), iter.key(), metaData.Homography);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vpFrameMap::getFrame(int frameIndex, vpFrame& frame)
{
  QTE_D();

  if (frameIndex >= d->DataSource->frames())
    {
    return false;
    }

  const auto& frameName = d->DataSource->frameName(frameIndex);

  synchronized (&d->Mutex)
    {
    const auto* const metaData = qtGet(d->ImageToMetaDataMap, frameName);
    if (!metaData)
      {
      return false;
      }

    const auto frameNum = static_cast<unsigned int>(frameIndex);
    frame.set(frameIndex, vgTimeStamp{metaData->Time, frameNum},
              metaData->Homography);
    }

  return true;
}

//-----------------------------------------------------------------------------
QMap<int, vgTimeStamp> vpFrameMap::timeMap()
{
  QTE_D();

  QMap<int, vgTimeStamp> out;

  synchronized (&d->Mutex)
    {
    for (const auto& iter : qtEnumerate(d->TimeToImageMap))
      {
      out.insert(iter.value(), iter.key());
      }
    }

  return out;
}

//-----------------------------------------------------------------------------
void vpFrameMap::setImageTime(const QString& filename, double microseconds)
{
  QTE_D();

  Q_ASSERT(!this->isRunning());
  if (auto* const metaData = qtGet(d->ImageToMetaDataMap, filename))
    {
    metaData->Time = microseconds;
    return;
    }
  d->ImageToMetaDataMap.insert(filename, {microseconds});
}

//-----------------------------------------------------------------------------
void vpFrameMap::setImageHomography(const QString& filename,
                                    vtkMatrix4x4 *homography)
{
  QTE_D();

  Q_ASSERT(!this->isRunning());
  if (auto* const metaData = qtGet(d->ImageToMetaDataMap, filename))
    {
    vpFrameMetaData::SetHomography(metaData->Homography, homography);
    return;
    }
  d->ImageToMetaDataMap.insert(filename, {homography});
}

//-----------------------------------------------------------------------------
void vpFrameMap::startUpdate()
{
  QTE_D();

  if (this->isRunning())
    {
    qDebug() << "Frame map update has already started.";
    return;
    }

  d->FrameNames = d->DataSource->frameNames();

  d->Stop = false;
  d->Progress = 0;
  this->start();
}

//-----------------------------------------------------------------------------
void vpFrameMap::stop()
{
  QTE_D();

  d->Stop = true;
  this->wait();
}

//-----------------------------------------------------------------------------
int vpFrameMap::progress()
{
  QTE_D();

  return d->Progress;
}

//-----------------------------------------------------------------------------
QList<QPair<QString, double>> vpFrameMap::imageTimes()
{
  QTE_D();

  QList<QPair<QString, double>> out;

  synchronized (&d->Mutex)
    {
    foreach (const auto& iter, qtEnumerate(d->ImageToMetaDataMap))
      {
      out.append({iter.key(), iter.value().Time});
      }
    }

  return out;
}

//-----------------------------------------------------------------------------
void vpFrameMap::run()
{
  QTE_D();

  synchronized (&d->Mutex)
    {
    d->TimeToImageMap.clear();
    }

  int numFiles = d->FrameNames.count();
  if (numFiles == 0)
    {
    return;
    }

  const auto& firstFrameName = d->FrameNames.first();
  vtkSmartPointer<vtkVgBaseImageSource> imageSource;
  imageSource.TakeReference(
    vpImageSourceFactory::GetInstance()->Create(stdString(firstFrameName)));

  if (!imageSource)
    {
    return;
    }

  foreach (const auto i, qtIndexRange(numFiles))
    {
    d->Progress = i;

    if (d->Stop)
      {
      return;
      }

    vtkVgTimeStamp ts;
    ts.SetFrameNumber(static_cast<unsigned int>(i));

    // Look up the frame name in our internal map to check if we've seen this
    // image before; If so, we don't need to read the timestamp metadata again
    const auto& frameName = d->FrameNames[i];
    synchronized (&d->Mutex)
      {
      if (auto* const entry = qtGet(d->ImageToMetaDataMap, frameName))
        {
        ts.SetTime(entry->Time);
        continue;
        }
      }

    imageSource->SetFileName(qPrintable(frameName));
    imageSource->UpdateInformation();

    const auto& imageTimeStamp = imageSource->GetImageTimeStamp();
    if (!imageTimeStamp.IsValid())
      {
      qDebug() << "vpFrameMap: Ignoring frame" << frameName
               << "(no timestamp data could be obtained)";
      continue;
      }

    ts.SetTime(imageTimeStamp.GetTime());
    vpFrameMetaData metaData{ts.GetTime()};

    synchronized (&d->Mutex)
      {
      d->ImageToMetaDataMap.insert(frameName, metaData);
      d->TimeToImageMap.insert(ts.GetRawTimeStamp(), i);
      }
    }

  // Set progress to number of frames to indicate completion
  d->Progress = numFiles;
}
