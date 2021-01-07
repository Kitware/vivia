// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpFseIO_h
#define __vpFseIO_h

#include "vpModelIO.h"

class vtkVgTrack;
class vtkVgEvent;

class vpFseIO : public vpModelIO
{
public:
  vpFseIO();
  virtual ~vpFseIO();

  virtual void SetTrackModel(vtkVpTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap);

  // not implemented
  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes);

  // not implemented
  virtual void SetActivityModel(vtkVgActivityManager* activityManager,
                                vpActivityConfig* activityConfig);

  void SetTracksFileName(const QString& tracksFileName);

  virtual void SetImageHeight(unsigned int imageHeight);
  virtual unsigned int GetImageHeight() const;

protected:
  unsigned int ImageHeight;
  QString TracksFilename;
};

#endif // __vpFseIO_h
