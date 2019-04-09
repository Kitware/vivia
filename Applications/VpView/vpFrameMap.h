/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFrameMap_h
#define __vpFrameMap_h

#include <vtkMatrix4x4.h>

#include <vtkVgTimeStamp.h>

#include <vgNamespace.h>

#include <qtGlobal.h>

#include <QThread>

// Forward declarations.
template <typename T> class QList;
template <typename T1, typename T2> struct QPair;
template <typename K, typename V> class QMap;

class vtkMatrix4x4;

class vpFileDataSource;
class vpFrameMapPrivate;

class vpFrame
{
public:
  vpFrame() = default;

  ~vpFrame()
    {
    if (this->Homography)
      {
      this->Homography->Delete();
      }
    }

  void set(int index, vgTimeStamp time,
           const vtkMatrix4x4* homography = nullptr);

  int Index = 0;
  vtkVgTimeStamp Time;
  vtkMatrix4x4* Homography = nullptr;
};

class vpFrameMap : public QThread
{
  Q_OBJECT

public:
  vpFrameMap(vpFileDataSource* dataSource);
  ~vpFrameMap();

  bool find(const vtkVgTimeStamp& time, vpFrame& frame,
            vg::SeekMode seekMode = vg::SeekNearest);

  bool isEmpty();

  bool first(vpFrame& frame);
  bool last(vpFrame& frame);
  bool getFrame(int frameIndex, vpFrame& frameValue);

  QMap<int, vgTimeStamp> timeMap();

  void setImageTime(const QString& filename, double microseconds);
  void setImageHomography(const QString& filename, vtkMatrix4x4* homography);
  void startUpdate();
  void stop();

  int progress();

  QList<QPair<QString, double>> imageTimes();

signals:
  void updated();

protected:
  virtual void run();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpFrameMap);

private:
  QTE_DECLARE_PRIVATE(vpFrameMap);
  Q_DISABLE_COPY(vpFrameMap);
};

#endif
