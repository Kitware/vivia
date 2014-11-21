/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqArchiveVideoSource_h
#define __vqArchiveVideoSource_h

// VG includes.
#include <vtkVgMacros.h>
#include <vtkVgVideoProviderBase.h>

// VV includes.
#include <vvIqr.h>

// STL includes.
#include <vector>

// QT includes.
#include <QObject>
#include <QSharedPointer>
#include <QUrl>

class vgKwaArchive;
class vgKwaFrameMetadata;
class vgKwaVideoClip;

class vqArchiveVideoSource : public QObject, public vtkVgVideoProviderBase
{
  Q_OBJECT

public:
  enum DeepCopyMode
    {
    // Description:
    // Permit sharing with the source instance's video clip. The clip will be
    // copied such that its state is independent of the source clip, but it may
    // share resources with the source clip. This is more efficient and uses
    // fewer system resources, but is not safe if the objects will be used in
    // different threads.
    CopyClipShared,
    // Description:
    // Do not allow sharing with the source instance's video clip. A new clip
    // will be created using the source clip's URI.
    CopyClipDetached
    };

  vtkVgClassMacro(vqArchiveVideoSource);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vqArchiveVideoSource, vtkVgVideoProviderBase);

  static vqArchiveVideoSource* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get and store a clip from a given ::vgKwaArchive.
  int AcquireVideoClip(vgKwaArchive* videoArchive);

  // Description:
  // Get and store a clip from a given URI.
  int AcquireVideoClip(QUrl);

  QUrl GetClipUri() const
    {
    return this->CurrentClipUri;
    }

  bool HasVideoClip() const
    {
    return !this->CurrentClip.isNull();
    }

  // Description:
  // Functions overridden.
  virtual int GetFrame(vtkVgVideoFrameData* frameData, double time);
  virtual int GetFrame(vtkVgVideoFrameData* frameData, int frameNumber);

  virtual int GetNextFrame(vtkVgVideoFrameData* frameData);
  virtual int GetCurrentFrame(vtkVgVideoFrameData* frameData);
  virtual int GetPreviousFrame(vtkVgVideoFrameData* frameData);

  virtual int GetNumberOfFrames();

  virtual int GetCurrentMetadata(vtkVgVideoMetadata* metadata);
  virtual std::map<vtkVgTimeStamp, vtkVgVideoMetadata> GetMetadata();

  virtual int GetVideoHeight();

  virtual const vgKwaVideoClip* GetCurrentVideoClip() const;

  virtual void Update();

  virtual int  Reset();

  virtual bool SeekNearestEarlier(double);

  virtual bool Advance();

  virtual bool Recede();

  virtual void ShallowCopy(vtkVgDataSourceBase& other);
  virtual void DeepCopy(vqArchiveVideoSource& other, DeepCopyMode);

signals:

  void frameAvailable();

protected:
  vqArchiveVideoSource();
  virtual  ~vqArchiveVideoSource();

  virtual void DeepCopy(vtkVgDataSourceBase& other);

  int SetVideoClip(vgKwaVideoClip*, const QUrl&);

  void CopyFrameData(vtkVgVideoFrameData* frameData);
  void CopyMetadata(vtkVgVideoMetadata* dst, const vgKwaFrameMetadata& src);

  int LastLoopingState;

  QUrl CurrentClipUri;
  QSharedPointer<vgKwaVideoClip> CurrentClip;

  vtkVgVideoFrameData* CurrentVideoFrameData;

private:
  vqArchiveVideoSource(const vqArchiveVideoSource&); // Not implemented.
  void operator= (const vqArchiveVideoSource&);      // Not implemented.
};

#endif // __vqArchiveVideoSource_h
