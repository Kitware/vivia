/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsQfVideoSource_h
#define __vsQfVideoSource_h

#include <QObject>

#include <vtkVgMacros.h>
#include <vtkVgVideoProviderBase.h>

#include <vtkVgVideoFrame.h>

class vgVideoPlayer;
class vgVideoSource;

class vsQfVideoSource : public QObject, public vtkVgVideoProviderBase
{
  Q_OBJECT

public:
  vtkVgClassMacro(vsQfVideoSource);

  vtkTypeMacro(vsQfVideoSource, vtkVgVideoProviderBase);

  static vsQfVideoSource* New();

  void SetSource(vgVideoSource*);

  void Play();
  void Pause();
  void Stop();

  virtual void SetTimeRange(double, double);
  virtual void SetTimeRange(double[2]);

  virtual int GetFrame(vtkVgVideoFrameData* frameData, double time);
  virtual int GetFrame(vtkVgVideoFrameData* frameData, int frameNumber);

  virtual int GetNextFrame(vtkVgVideoFrameData* frameData);
  virtual int GetCurrentFrame(vtkVgVideoFrameData* frameData);
  virtual int GetPreviousFrame(vtkVgVideoFrameData* frameData);

  virtual int GetNumberOfFrames();

  void SetMetadata(
    const std::map<vtkVgTimeStamp, vtkVgVideoMetadata>& allMetadata);

  virtual std::map<vtkVgTimeStamp, vtkVgVideoMetadata> GetMetadata();

  virtual int GetCurrentMetadata(vtkVgVideoMetadata* metadata);

  virtual int GetVideoHeight();

  virtual void Update();

  virtual int Reset();

  virtual bool Advance() { return false; }
  virtual bool Recede() { return false; }
  virtual bool SeekNearestEarlier(double) { return false; }

  virtual void ShallowCopy(vtkVgDataSourceBase& other);
  virtual void DeepCopy(vtkVgDataSourceBase& other);

signals:
  void frameAvailable();

protected slots:
  void updateVideoFrame(vtkVgVideoFrame, qint64 seekRequestId);

protected:
  vsQfVideoSource();
  virtual ~vsQfVideoSource();

  void UpdatePlayerFrameRange();

  vtkVgVideoFrameData* CurrentVideoFrameData;
  vtkVgVideoMetadata* CurrentVideoMetadata;

  std::map<vtkVgTimeStamp, vtkVgVideoMetadata> AllMetadata;

  vtkVgVideoFrame* Frame;
  QScopedPointer<vgVideoPlayer> Player;

  qint64 RequestId;

private:
  vsQfVideoSource(const vsQfVideoSource&); // Not implemented.
  void operator=(const vsQfVideoSource&); // Not implemented.
};

#endif
