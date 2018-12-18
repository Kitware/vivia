/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpModelIO_h
#define __vpModelIO_h

#include "vpActivityIO.h"
#include "vpEventIO.h"
#include "vpFseTrackIO.h"
#include "vpTrackIO.h"

#include <memory>
#include <vector>

class vgAttributeSet;
class vpFrameMap;

class vpModelIO
{
public:
  vpModelIO();
  virtual ~vpModelIO() {}

  virtual void SetTrackModel(vtkVpTrackModel* trackModel,
                             vpTrackIO::TrackStorageMode storageMode,
                             bool interpolateToGround,
                             vpTrackIO::TrackTimeStampMode timeStampMode,
                             vtkVgTrackTypeRegistry* trackTypes,
                             vgAttributeSet* trackAttributes,
                             vtkMatrix4x4* geoTransform,
                             vpFrameMap* frameMap) = 0;

  virtual void SetEventModel(vtkVgEventModel* eventModel,
                             vtkVgEventTypeRegistry* eventTypes) = 0;

  virtual void SetActivityModel(vtkVgActivityManager* activityManager,
                                vpActivityConfig* activityConfig) = 0;

  virtual void SetImageHeight(unsigned int imageHeight) = 0;
  virtual unsigned int GetImageHeight() const = 0;

  void SetTrackOverrideColor(const vgColor&);

  virtual bool ReadFrameMetaData(vpFrameMap* frameMap,
                                 const std::string& substitutePath);

  virtual int GetHomographyCount();
  virtual const std::vector<std::string>& GetImageFiles() const;

  bool ReadTracks(int frameOffset);
  bool ReadTrackTraits();
  bool ReadTrackClassifiers();

  bool ImportTracks(int frameOffset, vtkIdType idsOffset = 0,
                    float offsetX = 0.0f, float offsetY = 0.0f);

  bool WriteTracks(const char* filename, int frameOffset,
                   QPointF aoiOffset);

  bool ReadEvents();
  bool ReadEventLinks();

  bool ImportEvents(vtkIdType idsOffset = 0,
                    float offsetX = 0.0f, float offsetY = 0.0f);

  bool WriteEvents(const char* filename);

  bool ReadActivities();

  bool ReadFseTracks();
  bool ImportFseTracks(vtkIdType idsOffset = 0,
                       float offsetX = 0.0f, float offsetY = 0.0f);

  bool WriteFseTracks(const char* filename, QPointF aoiOffset,
                      bool writeSceneElements = true);

  const vpTrackIO* GetTrackIO() const { return this->TrackIO.get(); }
  const vpEventIO* GetEventIO() const { return this->EventIO.get(); }
  const vpActivityIO* GetActivityIO() const { return this->ActivityIO.get(); }
  const vpFseTrackIO* GetFseTrackIO() const { return this->FseTrackIO.get(); }

protected:
  std::unique_ptr<vpTrackIO> TrackIO;
  std::unique_ptr<vpEventIO> EventIO;
  std::unique_ptr<vpActivityIO> ActivityIO;
  std::unique_ptr<vpFseTrackIO> FseTrackIO;
};

#endif // __vpModelIO_h
