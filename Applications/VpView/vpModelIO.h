/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpModelIO_h
#define __vpModelIO_h

#include "vpTrackIO.h"
#include "vpEventIO.h"
#include "vpActivityIO.h"

#include <vector>

class vpFrameMap;
class vpFseTrackIO;

class vpModelIO
{
public:
  vpModelIO();
  virtual ~vpModelIO() {}

  virtual void SetTrackModel(vtkVgTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap) = 0;

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes) = 0;

  virtual void SetActivityModel(vtkVgActivityManager* activityManager,
                                vpActivityConfig* activityConfig) = 0;

  virtual void SetImageHeight(unsigned int imageHeight) = 0;
  virtual unsigned int GetImageHeight() const = 0;

  void SetTrackOverrideColor(double color[3]);

  virtual bool ReadFrameMetaData(vpFrameMap* frameMap,
                                 const std::string& substitutePath);

  virtual int GetHomographyCount();
  virtual const std::vector<std::string>& GetImageFiles() const;

  bool ReadTracks();
  bool ReadTrackTraits();
  bool ReadTrackPVOs();

  bool ImportTracks(vtkIdType idsOffset = 0,
                    float offsetX = 0.0f, float offsetY = 0.0f);

  bool WriteTracks(const char* filename);

  bool ReadEvents();
  bool ReadEventLinks();

  bool ImportEvents(vtkIdType idsOffset = 0,
                    float offsetX = 0.0f, float offsetY = 0.0f);

  bool WriteEvents(const char* filename);

  bool ReadActivities();

  bool ReadFseTracks();
  bool ImportFseTracks(vtkIdType idsOffset = 0,
                       float offsetX = 0.0f, float offsetY = 0.0f);

  bool WriteFseTracks(const char* filename, bool writeSceneElements = true);

  const vpTrackIO* GetTrackIO() const { return this->TrackIO; }
  const vpEventIO* GetEventIO() const { return this->EventIO; }
  const vpActivityIO* GetActivityIO() const { return this->ActivityIO; }
  const vpFseTrackIO* GetFseTrackIO() const { return this->FseTrackIO; }

protected:
  vpTrackIO* TrackIO;
  vpEventIO* EventIO;
  vpActivityIO* ActivityIO;
  vpFseTrackIO* FseTrackIO;
};

#endif // __vpModelIO_h
