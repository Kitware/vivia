/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFrameMap.h"

#include "vpFileDataSource.h"
#include "vpImageSourceFactory.h"

#include <vtkVgBaseImageSource.h>
#include <vtkVgTimeStamp.h>

#include <vgTimeMap.h>

#include <qtDebug.h>

#include <QAtomicInt>
#include <QMutex>
#include <QMutexLocker>

#include <map>
#include <string>

QTE_IMPLEMENT_D_FUNC(vpFrameMap)

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
  vpFrameMapPrivate() : Progress(0), Stop(false)
    {}

  vpFileDataSource* DataSource;
  vgTimeMap<unsigned int> TimeToImageMap;
  std::map<std::string, vpFrameMetaData> ImageToMetaDataMap;

  QAtomicInt Progress;
  volatile bool Stop;

  QMutex TimeToImageMapMutex;
};

//-----------------------------------------------------------------------------
void vpFrame::set(unsigned int index, vgTimeStamp time,
                  const vtkMatrix4x4* homography/* = 0*/)
{
  this->Index = index;
  this->Time = time;
  vpFrameMetaData::SetHomography(this->Homography, homography);
}

//-----------------------------------------------------------------------------
vpFrameMap::vpFrameMap(vpFileDataSource* dataSource) :
  d_ptr(new vpFrameMapPrivate)
{
  QTE_D(vpFrameMap);

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
  QTE_D(vpFrameMap);

  QMutexLocker ml(&d->TimeToImageMapMutex);

  vgTimeMap<unsigned int>::iterator itr =
    d->TimeToImageMap.find(time.GetRawTimeStamp(), seekMode);

  if (itr == d->TimeToImageMap.end())
    {
    return false;
    }

  // we need to find the entry in the image to metadata map - has to be there
  // because we build the TimeToImageMap from ImageToMetaDataMap
  std::map<std::string, vpFrameMetaData>::iterator image2DataItr =
    d->ImageToMetaDataMap.find(d->DataSource->getDataFile(itr.value()));
  frame.set(itr.value(), itr.key(), image2DataItr->second.Homography);

  return true;
}

//-----------------------------------------------------------------------------
bool vpFrameMap::isEmpty()
{
  QTE_D(vpFrameMap);

  QMutexLocker ml(&d->TimeToImageMapMutex);
  return d->TimeToImageMap.isEmpty();
}

//-----------------------------------------------------------------------------
bool vpFrameMap::first(vpFrame& frame)
{
  QTE_D(vpFrameMap);

  QMutexLocker ml(&d->TimeToImageMapMutex);

  if (d->TimeToImageMap.isEmpty())
    {
    return false;
    }

  vgTimeMap<unsigned int>::iterator itr = d->TimeToImageMap.begin();

  // we need to find the entry in the image to metadata map - has to be there
  // because we build the TimeToImageMap from ImageToMetaDataMap
  std::map<std::string, vpFrameMetaData>::iterator image2DataItr =
    d->ImageToMetaDataMap.find(d->DataSource->getDataFile(itr.value()));
  frame.set(itr.value(), itr.key(), image2DataItr->second.Homography);

  return true;
}

//-----------------------------------------------------------------------------
bool vpFrameMap::last(vpFrame& frame)
{
  QTE_D(vpFrameMap);

  QMutexLocker ml(&d->TimeToImageMapMutex);

  if (d->TimeToImageMap.isEmpty())
    {
    return false;
    }

  vgTimeMap<unsigned int>::iterator itr = --d->TimeToImageMap.end();

  // we need to find the entry in the image to metadata map - has to be there
  // because we build the TimeToImageMap from ImageToMetaDataMap
  std::map<std::string, vpFrameMetaData>::iterator image2DataItr =
    d->ImageToMetaDataMap.find(d->DataSource->getDataFile(itr.value()));
  frame.set(itr.value(), itr.key(), image2DataItr->second.Homography);

  return true;
}

//-----------------------------------------------------------------------------
bool vpFrameMap::getFrame(unsigned int frameIndex, vpFrame& frame)
{
  QTE_D(vpFrameMap);

  QMutexLocker ml(&d->TimeToImageMapMutex);

  if (frameIndex >= (unsigned int)d->DataSource->getFileCount())
    {
    return false;
    }

  std::map<std::string, vpFrameMetaData>::iterator image2DataItr =
    d->ImageToMetaDataMap.find(d->DataSource->getDataFile(frameIndex));
  if (image2DataItr == d->ImageToMetaDataMap.end())
    {
    return false;
    }

  frame.set(frameIndex, vgTimeStamp(image2DataItr->second.Time),
            image2DataItr->second.Homography);
  return true;
}

//-----------------------------------------------------------------------------
std::map<unsigned int, vgTimeStamp> vpFrameMap::getTimeMap()
{
  QTE_D(vpFrameMap);

  QMutexLocker ml(&d->TimeToImageMapMutex);

  std::map<unsigned int, vgTimeStamp> result;

  for (const auto& ts : d->TimeToImageMap.keys())
    {
    if (ts.HasFrameNumber())
      {
      result.emplace(ts.FrameNumber, ts);
      }
    }

  return result;
}

//-----------------------------------------------------------------------------
void vpFrameMap::setImageTime(const std::string& filename, double microseconds)
{
  QTE_D(vpFrameMap);

  Q_ASSERT(!this->isRunning());
  std::map<std::string, vpFrameMetaData>::iterator itr =
    d->ImageToMetaDataMap.find(filename);
  if (itr != d->ImageToMetaDataMap.end())
    {
    itr->second.Time = microseconds;
    return;
    }
  vpFrameMetaData metaData(microseconds);
  d->ImageToMetaDataMap.insert(std::make_pair(filename, metaData));
}

//-----------------------------------------------------------------------------
void vpFrameMap::setImageHomography(const std::string& filename,
                                    vtkMatrix4x4 *homography)
{
  QTE_D(vpFrameMap);

  Q_ASSERT(!this->isRunning());
  std::map<std::string, vpFrameMetaData>::iterator itr =
    d->ImageToMetaDataMap.find(filename);
  if (itr != d->ImageToMetaDataMap.end())
    {
    vpFrameMetaData::SetHomography(itr->second.Homography, homography);
    return;
    }
  vpFrameMetaData metaData(homography);
  d->ImageToMetaDataMap.insert(std::make_pair(filename, metaData));
}

//-----------------------------------------------------------------------------
void vpFrameMap::startUpdate()
{
  QTE_D(vpFrameMap);

  if (this->isRunning())
    {
    qDebug() << "Frame map update has already started.";
    return;
    }

  d->Stop = false;
  d->Progress = 0;
  this->start();
}

//-----------------------------------------------------------------------------
void vpFrameMap::stop()
{
  QTE_D(vpFrameMap);

  d->Stop = true;
  this->wait();
}

//-----------------------------------------------------------------------------
int vpFrameMap::progress()
{
  QTE_D(vpFrameMap);

  return d->Progress;
}

//-----------------------------------------------------------------------------
void vpFrameMap::exportImageTimes(
  std::vector<std::pair<std::string, double> >& imageTimes)
{
  QTE_D(vpFrameMap);

  // Stop updating momentarily to avoid any races on the image -> time map.
  bool wasRunning = this->isRunning();
  this->stop();

  // iterate to extract time
  std::map<std::string, vpFrameMetaData>::iterator itr =
    d->ImageToMetaDataMap.begin();
  for (; itr != d->ImageToMetaDataMap.end(); itr++)
    {
    imageTimes.push_back(
      std::pair<std::string, double>(itr->first, itr->second.Time));
    }

  // Restart decoding thread.
  if (wasRunning)
    {
    this->startUpdate();
    }
}

//-----------------------------------------------------------------------------
void vpFrameMap::run()
{
  QTE_D(vpFrameMap);

    {
    QMutexLocker ml(&d->TimeToImageMapMutex);
    d->TimeToImageMap.clear();
    }

  int numFiles = d->DataSource->getFileCount();
  if (numFiles == 0)
    {
    return;
    }

  vtkSmartPointer<vtkVgBaseImageSource> imageSource;
  imageSource.TakeReference(
    vpImageSourceFactory::GetInstance()->Create(
      d->DataSource->getDataFile(0)));

  if (!imageSource)
    {
    return;
    }

  std::map<std::string, vpFrameMetaData>::iterator lastInsertPos =
    d->ImageToMetaDataMap.begin();
  for (int i = 0; i < numFiles; ++i, d->Progress = i)
    {
    if (d->Stop)
      {
      return;
      }

    // Look up the filename in our internal map to check if we've seen this
    // image before. If so, we don't need to read the timestamp metadata again.
    std::string filename = d->DataSource->getDataFile(i);
    std::map<std::string, vpFrameMetaData>::iterator itr =
      d->ImageToMetaDataMap.find(filename);

    vtkVgTimeStamp ts;
    if (itr != d->ImageToMetaDataMap.end())
      {
      ts.SetTime(itr->second.Time);
      ts.SetFrameNumber(i);
      }
    else
      {
      imageSource->SetFileName(filename.c_str());
      imageSource->UpdateInformation();
      vtkVgTimeStamp imageTimeStamp = imageSource->GetImageTimeStamp();
      if (!imageTimeStamp.IsValid())
        {
        qDebug() << "No timestamp data found for" << filename.c_str()
                 << ", frame will be ignored";
        continue;
        }
      ts.SetTime(imageTimeStamp.GetTime());
      ts.SetFrameNumber(i);
      vpFrameMetaData metaData(ts.GetTime());
      lastInsertPos =
        d->ImageToMetaDataMap.insert(lastInsertPos,
                                     std::make_pair(filename, metaData));
      }
      {
      QMutexLocker ml(&d->TimeToImageMapMutex);
      d->TimeToImageMap.insert(ts.GetRawTimeStamp(), static_cast<unsigned>(i));
      }
    }
}
