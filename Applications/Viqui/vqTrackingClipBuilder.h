/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqTrackingClipBuilder_h
#define __vqTrackingClipBuilder_h

#include "vtkVQTrackingClip.h"

#include <vtkImageData.h>

#include <QMutexLocker>
#include <QThread>

#include <algorithm>
#include <vector>

typedef std::pair< int, vtkSmartPointer<vtkVQTrackingClip> > ClipElement;
typedef std::vector<ClipElement> ClipVector;

namespace
{

//-----------------------------------------------------------------------------
struct CompareClipDuration
{
  bool operator()(const ClipElement& c1, const ClipElement& c2)
    {
    return c1.second->GetDuration() < c2.second->GetDuration();
    }
};

}

//-----------------------------------------------------------------------------
class vqTrackingClipBuilder : public QThread
{
  Q_OBJECT

public:
  vqTrackingClipBuilder()
    : QThread(), ShutdownRequested(false)
    { }

  ~vqTrackingClipBuilder()
    {
    this->disconnect();
    this->Shutdown();
    this->wait();
    }

  void BuildClips(ClipVector::iterator start, ClipVector::iterator end)
    {
    // Note: Clips may not become available in the order they were submitted.
    // Shorter clips will be processed before longer ones.

    // If the build thread has the first vector locked, put incoming clips into
    // the 'backup'. Since the build thread only holds one of these locks at
    // a time, we can acquire one here without blocking. This is the only
    // function that modifies the clip vectors.
    if (this->ClipsMutex[0].tryLock())
      {
      this->Clips[0].insert(this->Clips[0].end(), start, end);
      std::sort(this->Clips[0].begin(), this->Clips[0].end(), CompareClipDuration());
      this->ClipsMutex[0].unlock();
      }
    else
      {
      QMutexLocker x(&this->ClipsMutex[1]);
      this->Clips[1].insert(this->Clips[1].end(), start, end);
      std::sort(this->Clips[1].begin(), this->Clips[1].end(), CompareClipDuration());
      }

    this->start();
    }

  void SkipClip(int id)
    {
    // Search for the clip and mark it to be skipped if found. There is no
    // race here since this function is called from the same thread as
    // BuildClips().
    for (int i = 0; i < 2; ++i)
      {
      for (ClipVector::iterator itr = this->Clips[i].begin(),
           end = this->Clips[i].end(); itr != end; ++itr)
        {
        if (itr->first == id)
          {
          QMutexLocker x(&this->ClipIdMutex);
          itr->first = -1;
          break;
          }
        }
      }
    }

  void Shutdown()
    {
    QMutexLocker x(&this->ShutdownMutex);
    this->ShutdownRequested = true;
    }

signals:
  void ClipAvailable(vtkImageData* clip, int index);

protected:
  virtual void run()
    {
    // Keep switching between the two input vectors until there are no new clips
    // left to process. This will usually complete in one iteration, but will
    // require more if new clips get added before processing completes.
    for (int i = 0; ; i = !i)
      {
      QMutexLocker x(&this->ClipsMutex[i]);

      if (this->Clips[i].empty())
        {
        break;
        }

      for (ClipVector::const_iterator itr = this->Clips[i].begin(),
           end = this->Clips[i].end(); itr != end; ++itr)
        {
          {
          QMutexLocker x(&this->ShutdownMutex);
          if (this->ShutdownRequested)
            {
            return;
            }
          }

        int id;
          {
          QMutexLocker x(&this->ClipIdMutex);
          id = itr->first;
          }

        if (id >= 0)
          {
          vtkImageData* output = itr->second->GetOutputImageData();

          // Caller will need to Delete() the image data once it has been
          // received. This is to ensure the data keeps a reference count> 0
          // between the time when the signal is emitted until it is received.
          output->Register(0);

          emit this->ClipAvailable(output, id);
          }
        }
      this->Clips[i].clear();
      }
    }

private:
  bool ShutdownRequested;

  ClipVector Clips[2];

  QMutex ShutdownMutex;
  QMutex ClipIdMutex;

  QMutex ClipsMutex[2];
};

#endif // __vqTrackingClipBuilder_h
