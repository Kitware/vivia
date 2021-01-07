// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
               vtkVgTrackTypeRegistry* trackTypes = 0,
               vtkMatrix4x4* geoTransform = 0,
               vpFrameMap* frameMap = 0);

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
