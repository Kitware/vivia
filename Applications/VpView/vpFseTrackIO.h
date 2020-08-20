/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFseTrackIO_h
#define __vpFseTrackIO_h

#include "vpTrackIO.h"

#include <QString>

class vtkVgTrack;

class vpFseTrackIO : public vpTrackIO
{
public:
  vpFseTrackIO(vtkVpTrackModel* trackModel,
               vpTrackIO::TrackStorageMode storageMode,
               bool interpolateToGround,
               vpTrackIO::TrackTimeStampMode timeStampMode,
               vtkVgTrackTypeRegistry* trackTypes = nullptr,
               vtkMatrix4x4* geoTransform = nullptr,
               vpFileDataSource* imageDataSource = nullptr,
               vpFrameMap* frameMap = nullptr);

  virtual ~vpFseTrackIO();

  void SetTracksFileName(const QString& tracksFileName)
    { this->TracksFileName = tracksFileName; }

  void SetImageHeight(unsigned int imageHeight)
    {
    this->ImageHeight = imageHeight;
    }

  unsigned int GetImageHeight() const
    {
    return this->ImageHeight;
    }

  virtual bool ReadTracks(int frameOffset);
  virtual bool ImportTracks(int frameOffset, vtkIdType idsOffset,
                            float offsetX, float offsetY);

  virtual bool WriteTracks(const QString& filename, int frameOffset,
                           QPointF aoiOffset, bool writeSceneElements) const;

  virtual QStringList GetSupportedFormats() const;
  virtual QString GetDefaultFormat() const;

private:
  QString TracksFileName;
  unsigned int ImageHeight;
};

#endif // __vpFseTrackIO_h
