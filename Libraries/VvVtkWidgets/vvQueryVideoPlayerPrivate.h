/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// Qt includes
#include <QFileInfo>
#include <QHash>
#include <QMap>
#include <QMultiHash>

// QtExtensions includes
#include <qtUtil.h>

// visgui includes
#include <vtkVgAdapt.h>
#include <vtkVgCompositeEventRepresentation.h>
#include <vtkVgEvent.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgTimeStamp.h>
#include <vtkVgTrack.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>
#include <vtkVgVideoMetadata.h>
#include <vtkVgVideoModel0.h>
#include <vtkVgVideoNode.h>

// VTK includes
#include <vtkIdList.h>
#include <vtkPoints.h>

#include "vvVideoPlayerPrivate.h"

class vvClipVideoRepresentation;

//-----------------------------------------------------------------------------
class vvQueryVideoPlayerPrivate : public vvVideoPlayerPrivate
{
public:
  struct TrackInfo
    {
    vvTrackId Id;
    vtkVgTrack* Track;
    QList<vtkIdType> Events;
    };

  struct RegionInfo
    {
    bool IsKeyframe;
    vtkIdType KeyframeId;
    QRect Region;
    };

  typedef QHash<vtkIdType, TrackInfo> TrackMap;
  typedef QHash<vtkIdType, vgRegionKeyframe> KeyframeMap;
  typedef QMap<vtkVgTimeStamp, RegionInfo> QueryRegionMap;

  typedef std::map<vtkVgTimeStamp, vtkVgVideoMetadata> VideoMetaDataMap;

  typedef TrackMap::const_iterator Trackiter;
  typedef KeyframeMap::const_iterator KeyframeIter;
  typedef QueryRegionMap::const_iterator QueryRegionIter;

  typedef VideoMetaDataMap::iterator VideoMetaDataIter;

  TrackMap Tracks;
  KeyframeMap Keyframes;
  QueryRegionMap QueryRegions;

  VideoMetaDataMap VideoMetaData;

  vtkSmartPointer<vtkVgVideoNode> VideoNode;
  vtkSmartPointer<vtkVgVideoModel0> VideoModel;
  vtkSmartPointer<vtkVgEventModel> EventModel;
  vtkSmartPointer<vtkVgTrackModel> TrackModel;

  vtkSmartPointer<vtkVgCompositeEventRepresentation> EventRepresentation;
  vtkSmartPointer<vtkVgEventRegionRepresentation> DefaultEventRepresentation;
  vtkSmartPointer<vtkVgEventRegionRepresentation> KeyframeRepresentation;
  vtkSmartPointer<vtkVgTrackRepresentation> TrackRepresentation;
  vtkSmartPointer<vvClipVideoRepresentation> VideoRepresentation;

  QColor DefaultEventColor;

  QColor PersonTrackColor;
  QColor VehicleTrackColor;
  QColor OtherTrackColor;
  QColor UnclassifiedTrackColor;
  bool ColorTracksByPVO;

  QList<vvDescriptor> Descriptors;
  QHash<vvTrackId, vtkIdType> TrackIdMap;

  bool UpdatePending;

  vtkIdType JumpToTrackId;
  vtkVgTimeStamp JumpToTrackTime;
};
