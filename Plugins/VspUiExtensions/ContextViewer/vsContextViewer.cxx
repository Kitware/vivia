/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsContextViewer.h"

// VisGUI includes
#include "vsContextViewerPlugin.h"
#include "vsContextInteractionCallback.h"

#include <vsCore.h>
#include <vsScene.h>

#include <vsDisplayInfo.h>

#include <vtkVgTerrainSource.h>

#include <vtkVgGeode.h>
#include <vtkVgTerrain.h>
#include <vtkVgTransformNode.h>
#include <vtkVgVideoViewer.h>

#include <vtkVgMarker.h>

#include <vtkVgQtUtil.h>

#include <vtkVgEvent.h>
#include <vtkVgEventTypeRegistry.h>
#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgSpaceConversion.h>
#include <vtkVgTimeStamp.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include <vgColor.h>
#include <vgDebug.h>
#include <vgFileDialog.h>
#include <vgGeoUtil.h>
#include <vgUnixTime.h>

#include <vgGeodesy.h>
#include <vgUtil.h>

#include <qtScopedValueChange.h>

// VTK includes
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderedAreaPicker.h>
#include <vtkSmartPointer.h>

// QT includes
#include <QApplication>
#include <QClipboard>
#include <QHelpEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QTimerEvent>
#include <QToolTip>

namespace // anonymous
{

enum EntityType
{
  TrackEntity,
  EventEntity,
};

typedef vsDisplayInfo (vsScene::*GetInfoMethod)(vtkIdType);

//-----------------------------------------------------------------------------
struct MarkerInfo
{
  vtkIdType EntityId;
  vtkSmartPointer<vtkVgMarker> Marker;
  vtkVgTimeStamp Time;
};
typedef QHash<vtkIdType, MarkerInfo> MarkerInfoMap;

//-----------------------------------------------------------------------------
struct LabelInfo
{
  vtkVgTimeStamp StartTime;
  vtkVgTimeStamp EndTime;
  QString Name;
  QString Note;
};
typedef QHash<vtkIdType, LabelInfo> LabelInfoMap;

//-----------------------------------------------------------------------------
vgPoint2d barycenter(const vgPolygon2d& points)
{
  vgPoint2d result;

  const size_t k = points.size();
  for (size_t i = 0; i < k; ++i)
    {
    const vgPoint2d& point = points[i];
    result.X += point.X;
    result.Y += point.Y;
    }

  // This will incidentally make the result (NaN, NaN) if there were no input
  // points (ki -> inf; 0 * ki -> NaN)... which is perfectly reasonable :-)
  const double ki = 1.0 / k;
  result.X *= ki;
  result.Y *= ki;

  return result;
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate barycenter(const vgGeocodedPoly& polygon)
{
  const auto& points = polygon.Coordinate;
  const size_t k = points.size();

  vgGeocodedCoordinate result(0.0, 0.0, k ? polygon.GCS : -1);

  for (size_t i = 0; i < k; ++i)
    {
    const auto& point = points[i];
    result.Northing += point.Northing;
    result.Easting += point.Easting;
    }

  // This will incidentally make the result (NaN, NaN) if there were no input
  // points (ki -> inf; 0 * ki -> NaN)... which is perfectly reasonable :-)
  const double ki = 1.0 / k;
  result.Northing *= ki;
  result.Easting *= ki;

  return result;
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate barycenter(vtkVgVideoFrameCorners corners)
{
  const double lat = 0.25 * (corners.LowerLeft.Latitude +
                             corners.LowerRight.Latitude +
                             corners.UpperLeft.Latitude +
                             corners.UpperRight.Latitude);
  const double lon = 0.25 * (corners.LowerLeft.Longitude +
                             corners.LowerRight.Longitude +
                             corners.UpperLeft.Longitude +
                             corners.UpperRight.Longitude);
  return vgGeocodedCoordinate(lat, lon, corners.GCS);
}

} // namespace <anonymous>

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsContextViewerPrivate

//-----------------------------------------------------------------------------
class vsContextViewerPrivate
{
public:
  vsContextViewerPrivate(vsContextViewer* q, vsCore* core, vsScene* scene);

  vtkSmartPointer<vtkVgMarker> addMarker(
    vtkIdType id, int type, const vtkVgTimeStamp& timeStamp,
    const vgPoint2d& stabPosition,
    const vgGeocodedCoordinate& geoPosition = vgGeocodedCoordinate());

  vtkSmartPointer<vtkVgMarker> addMarker(
    vtkIdType id, int type, const vtkVgTimeStamp& timeStamp,
    const vgGeocodedPoly& geoRegion);

  void finalizeMarker(vtkVgMarker* marker, vtkIdType id, int type);

  void setMarkerReadyPosition(
    vtkVgMarker* marker, const vgGeocodedCoordinate& gc);

  void setMarkerPosition(vtkVgMarker* marker, const vgGeocodedCoordinate& gc);
  bool setMarkerPosition(
    vtkVgMarker* marker, const vtkVgTimeStamp& timeStamp,
    const vgPoint2d& stabCenter,
    const vgGeocodedCoordinate& geoCenter = vgGeocodedCoordinate());

  void setMarkerReadyRegion(vtkVgMarker* marker,
                            const vgGeocodedPoly& geoRegion);
  void setMarkerRegion(vtkVgMarker* marker, const vgGeocodedPoly& geoRegion);

  void selectItemsOnContext();
  void pickItemOnContext();

  vtkVgMarker* markerAt(int pos[2]);
  vtkVgMarker* markerAt(double x, double y);

  void setSelection(const MarkerInfoMap& markers,
                    const QSet<vtkIdType> selectedIds);

  void deselect(const MarkerInfoMap& markers);

  QSet<vtkIdType> selectedMarkers(const MarkerInfoMap& markers);

  void selectTrack(vtkIdType eventId);
  void selectTracks(QSet<vtkIdType> eventIds);

  void selectEvent(vtkIdType eventId);
  void selectEvents(QSet<vtkIdType> eventIds);

  void updateTerrainSource();

  void updateMarker(const MarkerInfo&, GetInfoMethod);
  void updateMarkers(const MarkerInfoMap&, GetInfoMethod);

  QString getClassificationText(int type, vtkIdType id);
  void showToolTip(const QPoint& pos, vtkVgMarker* marker);

  vgGeocodedCoordinate displayToWorld(double x, double y);

protected:
  QTE_DECLARE_PUBLIC_PTR(vsContextViewer)

  bool SceneDirty;
  bool RenderPending;
  bool ModifyingSelection;

  double TerrainBounds[6];

  vtkSmartPointer<vtkVgVideoViewer> Viewer;
  vtkSmartPointer<vtkVgTransformNode> ContextSceneRoot;
  vtkSmartPointer<vtkVgTransformNode> SceneObjectsRoot;
  vtkSmartPointer<vtkVgTerrain> Terrain;
  vtkSmartPointer<vtkVgTerrainSource> TerrainSource;
  vtkSmartPointer<vtkVgInteractorStyleRubberBand2D> ContextInteractorStyle;

  vtkSmartPointer<vsContextInteractionCallback> InteractionCallback;

  MarkerInfoMap TrackMarkers;
  MarkerInfoMap EventMarkers;

  LabelInfoMap TrackInfo;
  LabelInfoMap EventInfo;

  QHash<vtkVgMarker*, vgGeocodedCoordinate> UnresolvedMarkers;
  QHash<vtkVgMarker*, vgGeocodedPoly> UnresolvedMarkerRegions;

  vsCore* const Core;
  vsScene* const Scene;

  int ClickTimerId;
  int LastClickPosition[2];
  int LastRightClickPosition[2];

private:
  QTE_DECLARE_PUBLIC(vsContextViewer)
};

QTE_IMPLEMENT_D_FUNC(vsContextViewer)

//-----------------------------------------------------------------------------
vsContextViewerPrivate::vsContextViewerPrivate(
  vsContextViewer* q, vsCore* core, vsScene* scene) :
  q_ptr(q),
  Core(core),
  Scene(scene)
{
  this->SceneDirty = false;
  this->RenderPending = false;
  this->ModifyingSelection = false;

  this->ClickTimerId = 0;

  vtkMath::UninitializeBounds(this->TerrainBounds);

  this->SceneObjectsRoot = vtkVgTransformNode::SmartPtr::New();
  this->SceneObjectsRoot->SetName("SceneObjectsRoot");

  this->ContextSceneRoot = vtkVgTransformNode::SmartPtr::New();
  this->ContextSceneRoot->SetName("ContextSceneRoot");
  this->ContextSceneRoot->AddChild(this->SceneObjectsRoot);

  this->InteractionCallback =
    vtkSmartPointer<vsContextInteractionCallback>::New();
  this->InteractionCallback->SetViewer(q);

  this->Viewer = vtkSmartPointer<vtkVgVideoViewer>::New();
  this->Viewer->SetSceneRoot(this->ContextSceneRoot);
  this->Viewer->SetRenderWindowInteractor(q->GetInteractor());
  q->SetRenderWindow(this->Viewer->GetRenderWindow());

  this->ContextInteractorStyle =
    vtkSmartPointer<vtkVgInteractorStyleRubberBand2D>::New();

  // Add observers
  this->ContextInteractorStyle->AddObserver(
    vtkCommand::InteractionEvent, this->InteractionCallback);

  this->ContextInteractorStyle->AddObserver(
    vtkVgInteractorStyleRubberBand2D::ZoomCompleteEvent,
    this->InteractionCallback);

  this->ContextInteractorStyle->SetRenderer(
    this->Viewer->GetSceneRenderer());
  this->ContextInteractorStyle->RubberBandSelectionWithCtrlKeyOn();

  q->GetInteractor()->SetInteractorStyle(this->ContextInteractorStyle);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgMarker> vsContextViewerPrivate::addMarker(
  vtkIdType id, int type, const vtkVgTimeStamp& timeStamp,
  const vgPoint2d& stabPosition, const vgGeocodedCoordinate& geoPosition)
{
  vtkSmartPointer<vtkVgMarker> marker(vtkSmartPointer<vtkVgMarker>::New());
  if (!this->setMarkerPosition(marker, timeStamp, stabPosition, geoPosition))
    {
    return vtkSmartPointer<vtkVgMarker>();
    }

  this->finalizeMarker(marker, id, type);

  return marker;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgMarker> vsContextViewerPrivate::addMarker(
  vtkIdType id, int type, const vtkVgTimeStamp& timeStamp,
  const vgGeocodedPoly& geoRegion)
{
  if (geoRegion.Coordinate.size() <= 2)
    {
    const auto& geoCenter = barycenter(geoRegion);
    return this->addMarker(id, type, timeStamp, vgPoint2d(), geoCenter);
    }

  vtkSmartPointer<vtkVgMarker> marker(vtkSmartPointer<vtkVgMarker>::New());
  this->setMarkerRegion(marker, geoRegion);

  this->finalizeMarker(marker, id, type);

  return marker;
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::finalizeMarker(vtkVgMarker* marker, vtkIdType id,
                                            int type)
{
  marker->SetRenderer(this->Viewer->GetSceneRenderer());
  marker->PickableOn();
  marker->SetUseBounds(true);
  marker->SetId(id);
  marker->SetType(type);

  vtkSmartPointer<vtkVgGeode> markerGeode = vtkSmartPointer<vtkVgGeode>::New();
  markerGeode->AddDrawable(marker);
  this->SceneObjectsRoot->AddChild(markerGeode);
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::setMarkerReadyPosition(
  vtkVgMarker* marker, const vgGeocodedCoordinate& gc)
{
  const vtkMatrix4x4* const xf =
    this->TerrainSource->GetCoordinateTransformMatrix();

  double pos[3] = { gc.Easting, gc.Northing, 1.0 };
  vtkVgApplyHomography(pos, *xf, pos);

  marker->SetMarkerPosition(pos);
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::setMarkerPosition(
  vtkVgMarker* marker, const vgGeocodedCoordinate& gc)
{
  if (!this->TerrainSource)
    {
    // If context is not loaded, use unconverted lat/lon coordinates for now,
    // and add to list for later repositioning
    double pos[3] = { gc.Easting, gc.Northing, 1.0 };
    marker->SetMarkerPosition(pos);
    this->UnresolvedMarkers.insert(marker, gc);
    }
  else
    {
    this->setMarkerReadyPosition(marker, gc);
    }
}

//-----------------------------------------------------------------------------
bool vsContextViewerPrivate::setMarkerPosition(
  vtkVgMarker* marker, const vtkVgTimeStamp& timeStamp,
  const vgPoint2d& stabCenter, const vgGeocodedCoordinate& geoCenter)
{
  if (geoCenter.GCS != -1)
    {
    // If a valid geodetic center was provided, just use that
    setMarkerPosition(marker, geoCenter);
    return true;
    }

  // Otherwise, compute the geodetic center from the stabilized center
  const vgGeocodedCoordinate gc =
    vgGeodesy::convertGcs(this->Core->stabToWorld(stabCenter, timeStamp),
                          vgGeodesy::LatLon_Wgs84);
  if (gc.GCS == -1)
    {
    qDebug() << "Failed to compute world position for time"
             << timeStamp.GetRawTimeStamp();
    return false;
    }

  setMarkerPosition(marker, gc);
  return true;
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::setMarkerReadyRegion(
  vtkVgMarker* marker, const vgGeocodedPoly& geoRegion)
{
  const vtkMatrix4x4* const xf =
    this->TerrainSource->GetCoordinateTransformMatrix();

  const auto& polyPoints = geoRegion.Coordinate;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkIdType numPoints = static_cast<vtkIdType>(polyPoints.size());
  points->SetNumberOfPoints(numPoints);

  for (vtkIdType i = 0; i < numPoints; ++i)
    {
    double pos[3] = { polyPoints[i].Easting, polyPoints[i].Northing, 1.0 };
    vtkVgApplyHomography(pos, *xf, pos);
    points->SetPoint(i, pos);
    }

  marker->SetRegionPoints(points);
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::setMarkerRegion(
  vtkVgMarker* marker, const vgGeocodedPoly& geoRegion)
{
  if (!this->TerrainSource)
    {
    // If context is not loaded, use unconverted lat/lon coordinates for now,
    // and add to list for later repositioning
    const auto& polyPoints = geoRegion.Coordinate;
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkIdType numPoints = static_cast<vtkIdType>(polyPoints.size());
    points->SetNumberOfPoints(numPoints);

    for (vtkIdType i = 0; i < numPoints; ++i)
      {
      points->SetPoint(i, polyPoints[i].Easting, polyPoints[i].Northing, 1.0);
      }
    marker->SetRegionPoints(points);

    this->UnresolvedMarkerRegions.insert(marker, geoRegion);
    }
  else
    {
    this->setMarkerReadyRegion(marker, geoRegion);
    }
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::updateMarker(
  const MarkerInfo& markerInfo, GetInfoMethod getInfo)
{
  const vsDisplayInfo& info = (this->Scene->*getInfo)(markerInfo.EntityId);

  const double* c = info.Color.value().array;
  markerInfo.Marker->SetVisibility(info.Visible);
  markerInfo.Marker->SetDefaultColor(c[0], c[1], c[2]);

  double sc[3];
  vgColor::fillArray(this->Scene->selectionColor(), sc);
  markerInfo.Marker->SetSelectionColor(sc[0], sc[1], sc[2]);
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::updateMarkers(
  const MarkerInfoMap& markers, GetInfoMethod getInfo)
{
  QTE_Q(vsContextViewer);

  Q_ASSERT(this->Scene);

  foreach (const MarkerInfo& info, markers)
    {
    if (info.Marker)
      {
      this->updateMarker(info, getInfo);
      }
    }
  q->update();
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::updateTerrainSource()
{
  if (this->TerrainSource)
    {
    this->TerrainSource->SetImageLevel(-1);

    // Multiplying by scaling factor of 4 to bring in
    // lower resolution for better performance
    this->TerrainSource->SetVisibleScale(4 * this->Viewer->GetCurrentScale());

    double viewExtents[4];
    this->Viewer->GetCurrentViewExtents(viewExtents);
    int extents[4] =
      {
      static_cast<int>(floor(viewExtents[0])),
      static_cast<int>(ceil(viewExtents[1])),
      static_cast<int>(floor(viewExtents[2])),
      static_cast<int>(ceil(viewExtents[3]))
      };

    this->TerrainSource->SetVisibleExtents(extents);
    this->TerrainSource->Update();
    }
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::selectItemsOnContext()
{
  QTE_Q(vsContextViewer);

  int* startScreenPos = this->ContextInteractorStyle->GetStartPosition();
  int* endScreenPos = this->ContextInteractorStyle->GetEndPosition();

  vtkRenderer* ren = this->Viewer->GetSceneRenderer();
  vtkSmartPointer<vtkRenderedAreaPicker> area =
    vtkSmartPointer<vtkRenderedAreaPicker>::New();
  int picked = area->AreaPick(startScreenPos[0], startScreenPos[1],
                              endScreenPos[0],   endScreenPos[1], ren);

  // Get starting selection set
  bool selecting = true;
  QSet<vtkIdType> selectedTrackIds;
  QSet<vtkIdType> selectedEventIds;
  if (q->GetInteractor()->GetControlKey())
    {
    selectedTrackIds = selectedMarkers(this->TrackMarkers);
    selectedEventIds = selectedMarkers(this->EventMarkers);
    }

  if (picked == 1)
    {
    // Determine if we are selecting or deselecting
    if (q->GetInteractor()->GetControlKey())
      {
      vtkVgMarker* const marker = this->markerAt(startScreenPos);
      if (marker)
        {
        selecting = !marker->IsSelected();
        }
      }

    vtkProp3DCollection* propCollection = area->GetProp3Ds();
    this->Viewer->GetRenderWindowInteractor()->SetPicker(area);

    // Update selection state for markers in selection area
    propCollection->InitTraversal();
    while (vtkProp3D* prop = propCollection->GetNextProp3D())
      {
      vtkVgMarker* marker = vtkVgMarker::SafeDownCast(prop);
      if (marker)
        {
        const bool isTrack = (marker->GetType() == TrackEntity);
        QSet<vtkIdType>& set = (isTrack ? selectedTrackIds : selectedEventIds);
        if (selecting)
          {
          set.insert(marker->GetId());
          }
        else
          {
          set.remove(marker->GetId());
          }
        }
      }
    }

  // Update selection
  this->selectTracks(selectedTrackIds);
  this->selectEvents(selectedEventIds);
}


//-----------------------------------------------------------------------------
void vsContextViewerPrivate::setSelection(
  const MarkerInfoMap& markers, const QSet<vtkIdType> selectedIds)
{
  if (this->ModifyingSelection)
    {
    return;
    }

  // First deselect selected markers on the context
  this->deselect(markers);

  foreach (vtkIdType id, selectedIds)
    {
    if (markers.contains(id))
      {
      const vtkSmartPointer<vtkVgMarker>& marker = markers[id].Marker;
      if (marker)
        {
        marker->Select();
        }
      }
    }

  QTE_Q(vsContextViewer);
  q->update();
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::deselect(const MarkerInfoMap& markers)
{
  foreach (const MarkerInfo& info, markers)
    {
    if (info.Marker)
      {
      info.Marker->Deselect();
      }
    }
}

//-----------------------------------------------------------------------------
QSet<vtkIdType> vsContextViewerPrivate::selectedMarkers(
  const MarkerInfoMap& markers)
{
  QSet<vtkIdType> selection;

  foreach (const MarkerInfo& info, markers)
    {
    if (info.Marker && info.Marker->IsSelected())
      {
      selection.insert(info.EntityId);
      }
    }

  return selection;
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::selectTrack(vtkIdType trackId)
{
  QSet<vtkIdType> trackIds;
  trackIds.insert(trackId);
  this->selectTracks(trackIds);
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::selectTracks(QSet<vtkIdType> trackIds)
{
  QTE_Q(vsContextViewer);

  q->setSelectedTracks(trackIds);
  qtScopedValueChange<bool> ms(this->ModifyingSelection, true);
  emit q->trackSelectionChanged(trackIds);
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::selectEvent(vtkIdType eventId)
{
  QSet<vtkIdType> eventIds;
  eventIds.insert(eventId);
  this->selectEvents(eventIds);
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::selectEvents(QSet<vtkIdType> eventIds)
{
  QTE_Q(vsContextViewer);

  q->setSelectedEvents(eventIds);
  qtScopedValueChange<bool> ms(this->ModifyingSelection, true);
  emit q->eventSelectionChanged(eventIds);
}

//-----------------------------------------------------------------------------
vtkVgMarker* vsContextViewerPrivate::markerAt(int pos[2])
{
  return this->markerAt(pos[0], pos[1]);
}

//-----------------------------------------------------------------------------
vtkVgMarker* vsContextViewerPrivate::markerAt(double x, double y)
{
  vtkVgNodeBase* const node = this->Viewer->Pick(x, y, 0.0);

  vtkVgGeode* const geode = vtkVgGeode::SafeDownCast(node);
  if (geode)
    {
    vtkPropCollection* const drawables = geode->GetActiveDrawables();
    drawables->InitTraversal();
    while (vtkProp* const prop = drawables->GetNextProp())
      {
      vtkVgMarker* const marker = vtkVgMarker::SafeDownCast(prop);
      if (marker)
        {
        return marker;
        }
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::pickItemOnContext()
{
  QTE_Q(vsContextViewer);

  int pointerPosition[2];
  this->ContextInteractorStyle->GetEndPosition(pointerPosition);
  vtkVgMarker* const marker = this->markerAt(pointerPosition);
  const bool multiSelect = !!q->GetInteractor()->GetControlKey();

  if (!marker)
    {
    if (!multiSelect)
      {
      // Single click should deselect
      this->selectTracks(QSet<vtkIdType>());
      this->selectEvents(QSet<vtkIdType>());
      }
    return;
    }

  // Get starting selection set
  QSet<vtkIdType> selectedTrackIds;
  QSet<vtkIdType> selectedEventIds;
  if (multiSelect)
    {
    selectedTrackIds = selectedMarkers(this->TrackMarkers);
    selectedEventIds = selectedMarkers(this->EventMarkers);
    }

  const bool isTrack = (marker->GetType() == TrackEntity);
  if (this->ClickTimerId != 0)
    {
    const int threshold = QApplication::startDragDistance();
    if (abs(pointerPosition[0] - this->LastClickPosition[0]) < threshold &&
        abs(pointerPosition[1] - this->LastClickPosition[1]) < threshold)
      {
      if (isTrack)
        {
        emit q->trackPicked(marker->GetId());
        }
      else
        {
        emit q->eventPicked(marker->GetId());
        }
      q->killTimer(this->ClickTimerId);
      this->ClickTimerId = 0;
      return;
      }
    }

  this->ClickTimerId = q->startTimer(QApplication::doubleClickInterval());

  this->LastClickPosition[0] = pointerPosition[0];
  this->LastClickPosition[1] = pointerPosition[1];

  // Single click will just select the marker (or toggle selection in case of
  // ctrl-click)
  QSet<vtkIdType>& set = (isTrack ? selectedTrackIds : selectedEventIds);
  if (multiSelect && marker->IsSelected())
    {
    set.remove(marker->GetId());
    }
  else
    {
    set.insert(marker->GetId());
    }

  // Update selection
  this->selectTracks(selectedTrackIds);
  this->selectEvents(selectedEventIds);
}

//-----------------------------------------------------------------------------
QString vsContextViewerPrivate::getClassificationText(int type, vtkIdType id)
{
  if (type == TrackEntity)
    {
    const vsDisplayInfo& info = this->Scene->trackDisplayInfo(id);
    switch (info.BestClassification)
      {
      case vtkVgTrack::Person:
        return QString("Person (%1)").arg(info.Confidence, 0, 'f', 4);
      case vtkVgTrack::Vehicle:
        return QString("Vehicle (%1)").arg(info.Confidence, 0, 'f', 4);
      case vtkVgTrack::Other:
        return QString("Other (%1)").arg(info.Confidence, 0, 'f', 4);
      case vtkVgTrack::Unclassified:
        return "Unclassified";
      default:
        return "(none)";
      }
    }
  else if (type == EventEntity)
    {
    const vsDisplayInfo& info = this->Scene->eventDisplayInfo(id);
    if (info.BestClassification == -1)
      {
      return QString("(none)");
      }
    const vgEventType& eventType =
      this->Core->eventTypeRegistry()->GetTypeById(info.BestClassification);
    return QString("%2 (%1)").arg(info.Confidence, 0, 'f', 4)
                             .arg(QString::fromLocal8Bit(eventType.GetName()));
    }
  else
    {
    // Should never happen
    return "(unknown)";
    }
}

//-----------------------------------------------------------------------------
void vsContextViewerPrivate::showToolTip(
  const QPoint& pos, vtkVgMarker* marker)
{
  QTE_Q(vsContextViewer);

  QString text =
    "<table>\n"
    "  <tr><td colspan=\"2\">\n"
    "    <span style=\"font: bold large;\">%1</span> %2\n"
    "  </td></tr>\n"
    "  <tr>\n"
    "    <td style=\"font: bold;\">Start Time: </td><td>%4</td>\n"
    "  </tr>\n"
    "  <tr>\n"
    "    <td style=\"font: bold;\">End Time: </td><td>%5</td>\n"
    "  </tr>\n"
    "</table>";

  const vtkIdType id = marker->GetId();
  const int type = marker->GetType();
  const LabelInfo& info =
    (type == TrackEntity ? this->TrackInfo[id] : this->EventInfo[id]);

  text = text.arg(info.Name, this->getClassificationText(type, id),
                  vgUnixTime(info.StartTime.GetTime()).timeString(),
                  vgUnixTime(info.EndTime.GetTime()).timeString());

  if (!info.Note.isEmpty())
    {
    text += QString("<p>%1</p>").arg(info.Note);
    }

  QToolTip::showText(pos, text, q);
}

//-----------------------------------------------------------------------------
vgGeocodedCoordinate vsContextViewerPrivate::displayToWorld(double x, double y)
{
  vgGeocodedCoordinate result;
  double pos[3] = { x, y, 0.0 };

  vtkVgSpaceConversion::DisplayToWorldNormalized(
    this->Viewer->GetSceneRenderer(), pos, pos);

  vtkVgInstance<vtkMatrix4x4> contextToLatLon(
    *this->TerrainSource->GetCoordinateTransformMatrix());
  contextToLatLon->Invert();

  vtkVgApplyHomography(pos, contextToLatLon, result.Easting, result.Northing);
  result.GCS = vgGeodesy::LatLon_Wgs84;
  return result;
}

//END vsContextViewerPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsContextViewer

//-----------------------------------------------------------------------------
vsContextViewer::vsContextViewer(vsCore* core, vsScene* scene, QWidget* parent,
                                 Qt::WindowFlags flag) :
  QVTKWidget(parent, flag),
  d_ptr(new vsContextViewerPrivate(this, core, scene))
{
  QTE_D(vsContextViewer);
  vtkConnect(d->ContextInteractorStyle,
             vtkVgInteractorStyleRubberBand2D::KeyPressEvent_R,
             this, SLOT(resetView()));

  vtkConnect(d->ContextInteractorStyle,
             vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
             this, SLOT(selectItemsOnContext()));

  vtkConnect(d->ContextInteractorStyle,
             vtkVgInteractorStyleRubberBand2D::LeftClickEvent,
             this, SLOT(pickItemOnContext()));

  vtkConnect(this->GetInteractor(),
             vtkCommand::RightButtonReleaseEvent,
             this, SLOT(showContextMenu()));
}

//-----------------------------------------------------------------------------
vsContextViewer::~vsContextViewer()
{
}

//-----------------------------------------------------------------------------
void vsContextViewer::addRasterLayer(const QUrl& uri)
{
  QTE_D(vsContextViewer);

  if (uri.scheme().toLower() == "file")
    {
    // Now initialize the terrain source as currently we are using
    // raster to generate a terrain.
    d->TerrainSource = vtkVgTerrainSource::SmartPtr::New();
    d->TerrainSource->SetDataSource(uri.toEncoded().constData());

    // Add the context to the main viewer scene.
    d->Terrain = d->TerrainSource->CreateTerrain();
    if (!d->Terrain)
      {
      QMessageBox::warning(0, "Context Viewer",
                           "A problem occurred trying to load the layer file.");
      return;
      }
    d->Terrain->SetNodeReferenceFrame(vtkVgNodeBase::ABSOLUTE_REFERENCE);
    d->ContextSceneRoot->AddChild(d->Terrain);
    }
  else
    {
    QMessageBox::critical(0, "Context Viewer",
                          "Non-local context files are not supported.");
    }

  // Update positions of any markers that were created before the layer was
  // added
  foreach_iter (auto, iter, d->UnresolvedMarkers)
    {
    d->setMarkerReadyPosition(iter.key(), iter.value());
    }
  d->UnresolvedMarkers.clear();
  foreach_iter (auto, iter, d->UnresolvedMarkerRegions)
    {
    d->setMarkerReadyRegion(iter.key(), iter.value());
    }
  d->UnresolvedMarkerRegions.clear();

  this->updateSceneGraph();

  // Cache the bounds of the context for future use (resetting view)
  static_cast<vtkVgNodeBase*>(d->Terrain)->GetBounds(d->TerrainBounds);

  this->resetView();
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateSceneGraph()
{
  QTE_D(vsContextViewer);
  d->Viewer->Frame(vtkVgTimeStamp(false));
  d->SceneDirty = false;
}

//-----------------------------------------------------------------------------
void vsContextViewer::render()
{
  QTE_D(vsContextViewer);

  if (d->SceneDirty)
    {
    this->updateSceneGraph();
    }

  d->RenderPending = false;
  d->Viewer->ForceRender(false);

  // Above isn't enough to force a repaint; also force the QVTKWidget to
  // repaint itself
  this->QVTKWidget::update();
}

//-----------------------------------------------------------------------------
void vsContextViewer::pickItemOnContext()
{
  QTE_D(vsContextViewer);
  d->pickItemOnContext();
}

//-----------------------------------------------------------------------------
void vsContextViewer::selectItemsOnContext()
{
  QTE_D(vsContextViewer);
  d->selectItemsOnContext();
}

//-----------------------------------------------------------------------------
bool vsContextViewer::event(QEvent* e)
{
  if (e->type() == QEvent::ToolTip)
    {
    QTE_D(vsContextViewer);

    QHelpEvent* const he = static_cast<QHelpEvent*>(e);
    vtkVgMarker* const marker = d->markerAt(he->x(), this->height() - he->y());
    if (marker)
      {
      d->showToolTip(he->globalPos(), marker);
      }
    }
  return QVTKWidget::event(e);
}

//-----------------------------------------------------------------------------
void vsContextViewer::timerEvent(QTimerEvent* e)
{
  QTE_D(vsContextViewer);
  if (e->timerId() == d->ClickTimerId)
    {
    this->killTimer(d->ClickTimerId);
    d->ClickTimerId = 0;
    }
}

//-----------------------------------------------------------------------------
void vsContextViewer::update()
{
  QTE_D(vsContextViewer);

  if (!d->RenderPending)
    {
    // Render at the end of the event loop, which is when everything should
    // be done updating and we will be ready to render of the scene.
    d->RenderPending = true;
    QMetaObject::invokeMethod(this, "render", Qt::QueuedConnection);
    }
}

//-----------------------------------------------------------------------------
void vsContextViewer::resetView()
{
  QTE_D(vsContextViewer);
  d->Viewer->ResetView();

  double bounds[6];
  d->Viewer->GetSceneRenderer()->ComputeVisiblePropBounds(bounds);

  // need to include full terrain, as we may be zoomed in to see only a portion
  if (vtkMath::AreBoundsInitialized(d->TerrainBounds))
    {
    vgExpandLowerBoundary(d->TerrainBounds[0], bounds[0]);
    vgExpandUpperBoundary(d->TerrainBounds[1], bounds[1]);
    vgExpandLowerBoundary(d->TerrainBounds[2], bounds[2]);
    vgExpandUpperBoundary(d->TerrainBounds[3], bounds[3]);
    }

  this->setViewToExtents(bounds);

  this->update();
}

//-----------------------------------------------------------------------------
void vsContextViewer::setViewToExtents(double extents[4])
{
  QTE_D(vsContextViewer);

  d->ContextInteractorStyle->ZoomToExtents(
    d->Viewer->GetSceneRenderer(), extents);
}

//-----------------------------------------------------------------------------
void vsContextViewer::loadContext()
{
  QString fileName = vgFileDialog::getOpenFileName(
                       this, "Layer file to add...", QString(),
                       "Kitware Structured Text (*.kst);;"
                       "Georegistered TIFF (*.tif *.tiff);;"
                       "All files (*)");
  if (!fileName.isEmpty())
    {
    QUrl uri = QUrl::fromLocalFile(fileName);
    this->addRasterLayer(uri);
    }
}

//-----------------------------------------------------------------------------
vtkVgInteractorStyleRubberBand2D* vsContextViewer::interactorStyle()
{
  QTE_D(vsContextViewer);
  return d->ContextInteractorStyle;
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateSources()
{
  QTE_D(vsContextViewer);
  d->updateTerrainSource();
  this->update();
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateTrackMarker(vtkVgTrack* track)
{
  QTE_D(vsContextViewer);

  const vtkIdType id = track->GetId();
  vtkVgTimeStamp ts = track->GetStartFrame();
  if (ts.IsValid())
    {
    double pos[2];
    if (track->GetClosestFramePt(ts, pos))
      {
      MarkerInfo& info = d->TrackMarkers[id];
      if (!info.Marker)
        {
        info.EntityId = id;
        info.Time = ts;
        info.Marker = d->addMarker(id, TrackEntity, ts,
                                   vgPoint2d(pos[0], pos[1]));
        if (!info.Marker)
          {
          return;
          }
        }
      else if (info.Time != ts)
        {
        info.Time = ts;
        d->setMarkerPosition(info.Marker, ts, vgPoint2d(pos[0], pos[1]));
        }

      d->updateMarker(info, &vsScene::trackDisplayInfo);
      d->SceneDirty = true;
      this->update();
      }
    }
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateEventMarker(vtkVgEvent* event)
{
  QTE_D(vsContextViewer);

  const vtkIdType id = event->GetId();
  vtkVgTimeStamp ts = event->GetStartFrame();

  vgPoint2d regionCenter;
  vgGeocodedCoordinate geoCenter;

  const auto& geoRegions = event->GetGeodeticRegions();
  vgGeocodedPoly geoRegion;

  // Determine geodetic location of the event
  if (!geoRegions.empty())
    {
    // If using geodetic regions, just use the first available one
    geoRegion = geoRegions.begin()->second;
    if (geoRegion.GCS == -1 || geoRegion.Coordinate.size() == 0)
      {
      qDebug() << "Invalid geodetic region - ignoring";
      geoRegion.GCS = -1;
      }
    else if (geoRegion.Coordinate.size() < 2)
      {
      geoCenter = barycenter(geoRegion);
      }
    }

  if (geoRegion.GCS == -1)
    {
    if (ts.IsValid())
      {
      // Try to determine image location; in this case we use the region at (or
      // the first region following) the nominal start of the event
      const auto& region = event->GetRegionAtOrAfter(ts);
      if (!region.empty())
        {
        // Use center of image bounding box... this might not be ideal (often
        // we use the midpoint of the bottom edge for such purposes), but our
        // geodetic location is likely to be highly approximate anyway...
        regionCenter = barycenter(region);
        }
      else
        {
        // No regions in the event; use the center of the video frame at the
        // start of the event
        const vtkVgVideoFrameMetaData md =
          d->Core->frameMetadata(ts, vg::SeekNearest);
        if (!md.AreCornerPointsValid())
          {
          // Oh, dear, there is no geoposition information for the frame; we
          // won't be able to set the marker position
          // TODO search for a valid frame over event's duration?
          qDebug() << "Failed to compute world position for time"
                   << ts.GetRawTimeStamp();
          return;
          }
        geoCenter = barycenter(md.WorldLocation);
        }
      }
    else
      {
      // The event start time stamp isn't valid
      // TODO use the first available region?
      return;
      }
    }

  MarkerInfo& info = d->EventMarkers[id];
  if (!info.Marker)
    {
    info.EntityId = id;
    info.Time = ts;
    // If we have a valid geoRegion and it is actually a region (and not just a
    // point, or two), make it transparent (current expectation is that this is
    // an "Alert")
    double opacity = 1.0;
    if (geoRegion.GCS != -1 && geoRegion.Coordinate.size() > 2)
      {
      QSettings settings;
      opacity = settings.value("RegionOpacity", 0.6).toDouble();
      info.Marker = d->addMarker(id, EventEntity, ts, geoRegion);
      }
    else
      {
      info.Marker = d->addMarker(id, EventEntity, ts, regionCenter, geoCenter);
      }

    if (!info.Marker)
      {
      return;
      }
    info.Marker->GetProperty()->SetOpacity(opacity);
    }
  else if (!ts.IsValid())
    {
    // Must be a geodetic region - ignore update (for now?)
    }
  else if (info.Time != ts)
    {
    info.Time = ts;
    d->setMarkerPosition(info.Marker, ts, regionCenter, geoCenter);
    }

  d->updateMarker(info, &vsScene::eventDisplayInfo);
  d->SceneDirty = true;
  this->update();
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateRegionlessEventMarker(vtkVgEvent* event)
{
  QTE_D(vsContextViewer);

  // Only call updateEventMarker if there is no region data and the StartTime
  // of the event has changed
  vtkVgTimeStamp ts = event->GetStartFrame();
  if (ts.IsValid() && event->GetNumberOfRegions() == 0)
    {
    MarkerInfo& info = d->EventMarkers[event->GetId()];
    if (info.Time != ts)
      {
      this->updateEventMarker(event);
      }
    }
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateTrackInfo(vtkVgTrack* track)
{
  QTE_D(vsContextViewer);

  // We have nothing meaningful to do if the track isn't started yet and
  // GetEndFrame complains rather loudly about querying for end frame when the
  // track hasn't been started
  if (!track->IsStarted())
    {
    return;
    }

  LabelInfo& info = d->TrackInfo[track->GetId()];

  info.StartTime = track->GetStartFrame();
  info.EndTime = track->GetEndFrame();

  const char* const name = track->GetName();
  info.Name =
    (name ? QString::fromLocal8Bit(name) : QString::number(track->GetId()));

  const char* const note = track->GetNote();
  info.Note = (note ? QString::fromLocal8Bit(note) : QString());
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateEventInfo(vtkVgEvent* event)
{
  QTE_D(vsContextViewer);

  LabelInfo& info = d->EventInfo[event->GetId()];

  info.StartTime = event->GetStartFrame();
  info.EndTime = event->GetEndFrame();

  info.Name = QString::number(event->GetId());

  const char* const note = event->GetNote();
  info.Note = (note ? QString::fromLocal8Bit(note) : QString());
}

//-----------------------------------------------------------------------------
void vsContextViewer::setSelectedTracks(QSet<vtkIdType> trackIds)
{
  QTE_D(vsContextViewer);
  d->setSelection(d->TrackMarkers, trackIds);
}

//-----------------------------------------------------------------------------
void vsContextViewer::setSelectedEvents(QSet<vtkIdType> eventIds)
{
  QTE_D(vsContextViewer);
  d->setSelection(d->EventMarkers, eventIds);
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateTrackMarkers()
{
  QTE_D(vsContextViewer);
  d->updateMarkers(d->TrackMarkers, &vsScene::trackDisplayInfo);
}

//-----------------------------------------------------------------------------
void vsContextViewer::updateEventMarkers()
{
  QTE_D(vsContextViewer);
  d->updateMarkers(d->EventMarkers, &vsScene::eventDisplayInfo);
}

//-----------------------------------------------------------------------------
QSize vsContextViewer::minimumSizeHint() const
{
  return QSize(150, 150);
}

//-----------------------------------------------------------------------------
void vsContextViewer::showContextMenu()
{
  QTE_D(vsContextViewer);

  if (!d->TerrainSource)
    {
    return;
    }

  vtkRenderWindowInteractor* const interactor =
    this->GetRenderWindow()->GetInteractor();

  int x, y;
  interactor->GetEventPosition(x, y);
  const QPoint menuLocation =
    this->mapToGlobal(QPoint(x, interactor->GetSize()[1] - y - 1));

  QMenu menu(this);
  QAction* const copyLocation = menu.addAction("&Copy Location");
  QAction* const choice = menu.exec(menuLocation);

  if (choice == copyLocation)
    {
    vgGeocodedCoordinate coord = d->displayToWorld(x, y);

    QString coordString = vgGeodesy::coordString(coord);
    QApplication::clipboard()->setText(
      coordString.left(coordString.indexOf('(') - 1));
    }
}

//END vsContextViewer
