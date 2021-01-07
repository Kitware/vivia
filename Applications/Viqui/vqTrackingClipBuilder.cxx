// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqTrackingClipBuilder.h"

#include <vtkImageData.h>

#include <qtGlobal.h>

#include <QMutexLocker>

#include <algorithm>

namespace // anonymous
{

using ClipElement = vqTrackingClipBuilder::ClipElement;

//-----------------------------------------------------------------------------
struct CompareClipDuration
{
  bool operator()(const ClipElement& c1, const ClipElement& c2)
  {
    return c1.second->GetDuration() < c2.second->GetDuration();
  }
};

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vqTrackingClipBuilder::vqTrackingClipBuilder()
{
}

//-----------------------------------------------------------------------------
vqTrackingClipBuilder::~vqTrackingClipBuilder()
{
  this->disconnect();
  this->Shutdown();
  this->wait();
}

//-----------------------------------------------------------------------------
void vqTrackingClipBuilder::BuildClips(
  ClipVector::iterator start, ClipVector::iterator end)
{
  static CompareClipDuration comparator;

  // Note: Clips may not become available in the order they were submitted.
  // Shorter clips will be processed before longer ones.

  // If the build thread has the first vector locked, put incoming clips into
  // the 'backup'. Since the build thread only holds one of these locks at
  // a time, we can acquire one here without blocking. This is the only
  // function that modifies the clip vectors.
  if (this->ClipsMutex[0].tryLock())
  {
    this->Clips[0].insert(this->Clips[0].end(), start, end);
    std::sort(this->Clips[0].begin(), this->Clips[0].end(), comparator);
    this->ClipsMutex[0].unlock();
  }
  else
  {
    synchronized (&this->ClipsMutex[1])
    {
      this->Clips[1].insert(this->Clips[1].end(), start, end);
      std::sort(this->Clips[1].begin(), this->Clips[1].end(), comparator);
    }
  }

  this->start();
}

//-----------------------------------------------------------------------------
void vqTrackingClipBuilder::SkipClip(int id)
{
  // Search for the clip and mark it to be skipped if found. There is no race
  // here since this function is called from the same thread as BuildClips().
  for (auto& clips : this->Clips)
  {
    for (auto& clip : clips)
    {
      if (clip.first == id)
      {
        synchronized (&this->ClipIdMutex)
        {
          clip.first = -1;
        }
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void vqTrackingClipBuilder::Shutdown()
{
  this->ShutdownRequested = true;
}

//-----------------------------------------------------------------------------
void vqTrackingClipBuilder::run()
{
  // Keep switching between the two input vectors until there are no new clips
  // left to process. This will usually complete in one iteration, but will
  // require more if new clips get added before processing completes.
  for (int i = 0; ; i = !i)
  {
    synchronized (&this->ClipsMutex[i])
    {
      if (this->Clips[i].empty())
      {
        break;
      }

      for (const auto& clip : this->Clips[i])
      {
        if (this->ShutdownRequested)
        {
          return;
        }

        int id;
        synchronized (&this->ClipIdMutex)
        {
          id = clip.first;
        }

        if (id >= 0)
        {
          vtkImageData* output = clip.second->GetOutputImageData();

          // Caller will need to Delete() the image data once it has been
          // received. This is to ensure the data keeps a reference count > 0
          // between the time when the signal is emitted until it is received.
          output->Register(0);

          emit this->ClipAvailable(output, id);
        }
      }

      this->Clips[i].clear();
    }
  }
}
