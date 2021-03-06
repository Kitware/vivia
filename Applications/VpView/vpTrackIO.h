// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpTrackIO_h
#define __vpTrackIO_h

#include <vgColor.h>

#include <vtkSmartPointer.h>

#include <QPointF>

class vpFrameMap;

class vtkVpTrackModel;

class vtkVgTimeStamp;
class vtkVgTrack;
class vtkVgTrackTypeRegistry;

class vtkMatrix4x4;

class vpTrackIO
{
protected:
  enum TimeStampFlag
    {
    TSF_Time        = 1 << 0,
    TSF_FrameNumber = 1 << 1
    };

public:
  enum TrackStorageMode
    {
    TSM_ImageCoords,
    TSM_InvertedImageCoords,
    TSM_TransformedGeoCoords,
    TSM_HomographyTransformedImageCoords
    };

  enum TrackTimeStampMode
    {
    TTM_TimeOnly           = TSF_Time,
    TTM_FrameNumberOnly    = TSF_FrameNumber,
    TTM_TimeAndFrameNumber = TSF_Time | TSF_FrameNumber
    };

public:
  vpTrackIO(vtkVpTrackModel* trackModel,
            TrackStorageMode storageMode,
            bool interpolateToGround,
            TrackTimeStampMode timeStampMode,
            vtkVgTrackTypeRegistry* trackTypes,
            vtkMatrix4x4* geoTransform,
            vpFrameMap* frameMap);

  virtual ~vpTrackIO();

  void SetOverrideColor(const vgColor&);

  virtual bool ReadTracks(int frameOffset) = 0;
  virtual bool ReadTrackTraits();
  virtual bool ReadTrackClassifiers();

  virtual bool ImportTracks(int frameOffset, vtkIdType idsOffset,
                            float offsetX, float offsetY);

  virtual bool WriteTracks(
    const QString& filename, int frameOffset, QPointF aoiOffset,
    bool writeSceneElements) const = 0;

  // Return list of supported output formats as UI strings (e.g. "Foo (*.foo)")
  virtual QStringList GetSupportedFormats() const = 0;

  // Return default output format (i.e. extension without '.')
  virtual QString GetDefaultFormat() const = 0;

  virtual bool GetNextValidTrackFrame(vtkVgTrack* track,
                                      unsigned int startFrame,
                                      vtkVgTimeStamp& timeStamp) const;

  virtual bool GetPrevValidTrackFrame(vtkVgTrack* track,
                                      unsigned int startFrame,
                                      vtkVgTimeStamp& timeStamp) const;

  virtual vtkIdType GetModelTrackId(unsigned int sourceId) const;

  static void GetDefaultTrackColor(int trackId, double (&color)[3]);

  int GetTrackTypeIndex(const char* typeName);

protected:
  virtual unsigned int GetImageHeight() const = 0;

protected:
  friend class vpFileTrackReader;

  void AddTrack(vtkVgTrack*);

  vtkVpTrackModel* TrackModel;
  vtkVgTrackTypeRegistry* TrackTypes;

  TrackStorageMode StorageMode;
  TrackTimeStampMode TimeStampMode;
  vtkSmartPointer<vtkMatrix4x4> GeoTransform;
  vpFrameMap* FrameMap;
  vgColor OverrideColor;
  bool InterpolateToGround;
};

#endif // __vpTrackIO_h
