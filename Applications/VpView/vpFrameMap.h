/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
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

#include <vector>

// Forward declarations.
class vtkMatrix4x4;

class vpFileDataSource;
class vpFrameMapPrivate;

class vpFrame
{
public:
  vpFrame() : Index(0), Homography(0)
    {}

  ~vpFrame()
    {
    if (this->Homography)
      {
      this->Homography->Delete();
      }
    }

  void set(unsigned int index, vgTimeStamp time,
           const vtkMatrix4x4* homography = 0);

  unsigned int Index;
  vtkVgTimeStamp Time;
  vtkMatrix4x4* Homography;
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
  bool getFrame(unsigned int frameIndex, vpFrame& frameValue);

  void setImageTime(const std::string& filename, double microseconds);
  void setImageHomography(const std::string& filename,
                          vtkMatrix4x4 *homography);
  void startUpdate();
  void stop();

  int progress();

  void exportImageTimes(std::vector<std::pair<std::string, double> >&
                          imageTimes);

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
