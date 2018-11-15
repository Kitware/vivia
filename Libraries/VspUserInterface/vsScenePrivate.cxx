/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsScenePrivate.h"

#include <QSettings>

#include <vtkAssembly.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkImageStencil.h>
#include <vtkImplicitSelectionLoop.h>
#include <vtkImplicitFunctionToImageStencil.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <vtkVgContourOperatorManager.h>
#include <vtkVgEventFilter.h>
#include <vtkVgEventLabelRepresentation.h>
#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgEventRepresentation.h>
#include <vtkVgSpaceConversion.h>
#include <vtkVgTrackFSOFilter.h>
#include <vtkVgTrackHeadRepresentation.h>
#include <vtkVgTrackLabelRepresentation.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>
#include <vtkVgUtil.h>

#include <vgGeoUtil.h>

#include <vgMixerWidget.h>

#include "vsContourWidget.h"
#include "vsCore.h"
#include "vsEventTreeModel.h"
#include "vsScene.h"
#include "vsSettings.h"
#include "vsTrackInfo.h"
#include "vsTrackTreeModel.h"

//BEGIN vsSceneTrackColorHelper

//-----------------------------------------------------------------------------
class vsSceneTrackColorHelper : public vsAbstractSceneTrackColorHelper,
                                public vtkVgTrackColorHelper,
                                public vtkVgTrackLabelColorHelper
{
public:
  vsSceneTrackColorHelper(vsCore* core) : Core(core) {}
  virtual ~vsSceneTrackColorHelper() {}

  // vsAbstractSceneTrackColorHelper interface
  virtual const vsTrackInfo* infoForTrack(vtkVgTrack* track) const QTE_OVERRIDE;

  // vtkVgTrackColorHelper interface
  virtual const double* GetTrackColor(
    vtkVgTrackInfo track, double scalar) QTE_OVERRIDE;

  // vtkVgTrackLabelColorHelper interface
  virtual const double* GetTrackLabelBackgroundColor(
    vtkVgTrack* track) QTE_OVERRIDE;
  virtual const double* GetTrackLabelForegroundColor(
    vtkVgTrack* track) QTE_OVERRIDE;

  // Public members
  QHash<int, vsTrackInfo> SourceColors;

protected:

  vsCore* const Core;
};

//-----------------------------------------------------------------------------
const double* vsSceneTrackColorHelper::GetTrackColor(
  vtkVgTrackInfo track, double scalar)
{
  Q_UNUSED(scalar)
  const vsTrackInfo* const ti = this->infoForTrack(track.GetTrack());
  return (ti ? ti->pcolor : 0);
}

//-----------------------------------------------------------------------------
const double* vsSceneTrackColorHelper::GetTrackLabelBackgroundColor(
  vtkVgTrack* track)
{
  const vsTrackInfo* const ti = this->infoForTrack(track);
  return (ti ? ti->bcolor : 0);
}

//-----------------------------------------------------------------------------
const double* vsSceneTrackColorHelper::GetTrackLabelForegroundColor(
  vtkVgTrack* track)
{
  const vsTrackInfo* const ti = this->infoForTrack(track);
  return (ti ? ti->fcolor : 0);
}

//-----------------------------------------------------------------------------
const vsTrackInfo* vsSceneTrackColorHelper::infoForTrack(
  vtkVgTrack* track) const
{
  const vvTrackId tid = this->Core->logicalTrackId(track->GetId());

  const QHash<int, vsTrackInfo>::const_iterator iter =
    this->SourceColors.find(tid.Source);
  return (iter == this->SourceColors.constEnd() ? 0 : &iter.value());
}

//END vsSceneTrackColorHelper

///////////////////////////////////////////////////////////////////////////////

//BEGIN static helper methods

namespace // anonymous
{

//-----------------------------------------------------------------------------
double blend(double t, double a, double b)
{
  return ((1.0 - t) * a) + (t * b);
}

//-----------------------------------------------------------------------------
double blend2d(double u, double v, double tl, double tr, double bl, double br)
{
  const double l = blend(v, tl, bl), r = blend(v, tr, br);
  return blend(u, l, r);
}

//-----------------------------------------------------------------------------
void setTrackTypeColors(vsScenePrivate::Graph& graph, vsTrackInfo ti)
{
  vtkVgTrack::enumTrackFSOType type =
    static_cast<vtkVgTrack::enumTrackFSOType>(ti.type);
  graph.TrackRepresentation->SetColor(type, ti.pcolor);
  graph.TrackHeadRepresentation->SetColor(type, ti.pcolor);
  graph.TrackLabelRepresentation->SetTrackTypeColors(
    type, ti.bcolor, ti.fcolor);
}

//-----------------------------------------------------------------------------
void setDefaultColors(vsScenePrivate::Graph& graph, double (&color)[3])
{
  graph.TrackRepresentation->SetDefaultColor(color);
  graph.TrackHeadRepresentation->SetDefaultColor(color);
}

//-----------------------------------------------------------------------------
void setHelperForGraph(vsScenePrivate::Graph& graph,
                       vsSceneTrackColorHelper* helper)
{
  graph.TrackRepresentation->SetColorHelper(helper);
  graph.TrackHeadRepresentation->SetColorHelper(helper);
  graph.TrackLabelRepresentation->SetLabelColorHelper(helper);
  graph.TrackRepresentation->Modified();
  graph.TrackHeadRepresentation->Modified();
  graph.TrackLabelRepresentation->Modified();
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vsSceneTrackColorHelper* updateTrackColorHelper(
  vsSceneTrackColorHelper* helper)
{
  if (!vsSettings().colorTracksBySource())
    {
    // If per-source coloring is disabled, we don't really need to do anything,
    // and we return the helper that should be used (i.e. 'none' in this case)
    return 0;
    }

  // Otherwise, ensure that the helper's collection of source colors is up to
  // date...
  helper->SourceColors.clear();
  foreach (const vsTrackInfo& ti, vsTrackInfo::trackSources())
    {
    helper->SourceColors.insert(ti.source, ti);
    }

  // ...and return the helper
  return helper;
}

//END static helper methods

///////////////////////////////////////////////////////////////////////////////

//BEGIN vsScenePrivate

//-----------------------------------------------------------------------------
vsScenePrivate::vsScenePrivate(vsScene* q, vsCore* core) :
  q_ptr(q),
  Core(core),
  VideoSource(0),
  TrackColorHelper(new vsSceneTrackColorHelper(core)),
  SelectionColor(vsSettings().selectionPenColor()),
  FilteringMaskColor(vsSettings().filteringMaskColor()),
  GroundTruthEnabled(true),
  RenderWidget(0),
  FilterWidget(0),
  TrackTreeModel(0),
  EventDataModel(0),
  EventTreeModel(0),
  NeedViewReset(true),
  PendingJumpTrackId(-1),
  PendingJumpEventId(-1),
  WriteRenderedImages(false),
  ImageCounter(1),
  SaveScreenShot(false)
{
  // Initialize models
  this->initializeGraph(this->NormalGraph);
  this->initializeGraph(this->GroundTruthGraph, "GT-");

  core->addModel(this->NormalGraph.TrackModel);
  core->addModel(this->NormalGraph.EventModel);
  core->addModel(this->GroundTruthGraph.TrackModel, vsCore::GroundTruthModel);
  core->addModel(this->GroundTruthGraph.EventModel, vsCore::GroundTruthModel);

  // Initialize filters
  this->TrackFilter->SetShowType(vtkVgTrack::Fish, true);
  this->TrackFilter->SetMinProbability(vtkVgTrack::Fish, 0.0);
  this->TrackFilter->SetMaxProbability(vtkVgTrack::Fish, 1.0);
  this->TrackFilter->SetShowType(vtkVgTrack::Scallop, true);
  this->TrackFilter->SetMinProbability(vtkVgTrack::Scallop, 0.0);
  this->TrackFilter->SetMaxProbability(vtkVgTrack::Scallop, 1.0);
  this->TrackFilter->SetShowType(vtkVgTrack::Other, true);
  this->TrackFilter->SetMinProbability(vtkVgTrack::Other, 0.0);
  this->TrackFilter->SetMaxProbability(vtkVgTrack::Other, 1.0);

  // Set up track labels and coloring
  this->updateTrackColors();

  this->TrackingMask->GetProperty()->SetOpacity(
    this->FilteringMaskColor.constData().color.alpha);
  this->TrackingMask->SetInterpolate(0);

  QSettings settings;
  this->MaxTimeForQuickNote =
    settings.value("MaxQuickNoteTime", 1.0).toDouble();
}

//-----------------------------------------------------------------------------
vsScenePrivate::~vsScenePrivate()
{
}

//-----------------------------------------------------------------------------
const vsAbstractSceneTrackColorHelper* vsScenePrivate::trackColorHelper() const
{
  return this->TrackColorHelper.data();
}

//-----------------------------------------------------------------------------
void vsScenePrivate::createFilterEventGroup(
  const QString& name, vsEventInfo::Group group,
  vsEventInfo::Group parent, bool expanded)
{
  bool checkable = (group != vsEventInfo::All);
  int parentId = this->FilterGroup.value(parent, 0);
  int groupId = this->FilterWidget->addGroup(name, checkable, parentId);
  this->FilterGroup.insert(group, groupId);
  this->FilterWidget->setExpanded(groupId, expanded);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::createFilterEventGroup(
  const QString& name, vsEventInfo::Group group, bool expanded)
{
  this->createFilterEventGroup(name, group, vsEventInfo::All, expanded);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::setupFilterEventGroup(
  vsEventInfo::Group group, bool visibility, double threshold)
{
  const QList<vsEventInfo> eventTypes = vsEventInfo::events(group);
  this->setupFilterEventGroup(eventTypes, group, visibility, threshold);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::setupFilterEventGroup(
  const QList<vsEventInfo>& eventTypes, vsEventInfo::Group group,
  bool visibility, double threshold)
{
  int groupId = this->FilterGroup.value(group, 0);

  foreach (const vsEventInfo& ei, eventTypes)
    this->FilterWidget->addItem(ei.type, ei.name, groupId);
  this->FilterWidget->setGroupState(groupId, visibility);
  this->FilterWidget->setGroupValue(groupId, threshold);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::setFilterEventGroupVisibility(vsEventInfo::Group group)
{
  int groupId = this->FilterGroup.value(group, 0);
  bool filterVisibility = this->Core->isEventGroupExpected(group);
  this->FilterWidget->setGroupVisible(groupId, filterVisibility);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::addFilter(const vsEventInfo& ei, vsEventInfo::Group group,
                               bool visibility, double threshold)
{
  int groupId = this->FilterGroup.value(group, 0);
  this->FilterWidget->addItem(ei.type, ei.name, groupId);
  this->FilterWidget->setState(ei.type, visibility);
  this->FilterWidget->setValue(ei.type, threshold);
  this->EventFilter->SetShowType(ei.type, visibility);
  this->EventFilter->SetMinProbability(ei.type, threshold);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::createMaskActor(
  ContourInfo& info, const vgColor& fcolor, const vgColor& bcolor)
{
  vtkImplicitSelectionLoop* loop =
    this->ContourOperatorManager->GetContourLoop(info.widget->points());
  vtkPoints* points = loop->GetLoop();

  double bounds[6];
  points->GetBounds(bounds);
  vtkSmartPointer<vtkImageData> input =
    vsScenePrivate::createFilterMaskImage(bounds);

  vtkImplicitFunctionToImageStencil* loopToStencil =
    vtkImplicitFunctionToImageStencil::New();

  loopToStencil->SetInput(loop);
  loopToStencil->SetOutputOrigin(input->GetOrigin());
  // set output extents
  int ib[4] =
    {
    vtkMath::Floor(bounds[0]), vtkMath::Ceil(bounds[1]),
    vtkMath::Floor(bounds[2]), vtkMath::Ceil(bounds[3])
    };
  int dim[3] = { ib[1] - ib[0], ib[3] - ib[2], 1 };
  loopToStencil->SetOutputWholeExtent(
    0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  loopToStencil->Update();

  vtkImageStencil* stencil = vtkImageStencil::New();
  stencil->SetStencilConnection(loopToStencil->GetOutputPort());
  stencil->SetInputData(input);
  stencil->SetBackgroundValue(1.0);
  stencil->Update();

  info.maskActor = vtkSmartPointer<vtkImageActor>::New();
  info.maskActor->GetMapper()->SetInputConnection(
    stencil->GetOutputPort());

  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(2);
  lut->SetTableValue(0, fcolor.value().array); // foreground
  lut->SetTableValue(1, bcolor.value().array); // background
  lut->SetTableRange(0.0, 1.0);
  lut->SetRampToLinear();
  info.maskActor->GetProperty()->SetLookupTable(lut);
  info.maskActor->GetProperty()->UseLookupTableScalarRangeOn();

  info.maskActor->SetInterpolate(0);
  info.maskActor->Update();

  loopToStencil->FastDelete();
  stencil->FastDelete();
  lut->FastDelete();
}

//-----------------------------------------------------------------------------
void vsScenePrivate::createFilterMask(ContourInfo& info)
{
  this->createMaskActor(info, this->FilteringMaskColor, vgColor());
  this->FilterMaskProps->AddPart(info.maskActor);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::removeFilterMask(ContourInfo& info)
{
  this->FilterMaskProps->RemovePart(info.maskActor);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::createSelectorMask(ContourInfo& info)
{
  // Create a gray quad to cover the whole image; individual selectors will
  // mask out their respective parts of this with the help of stencil testing
  if (!this->SelectorMaskQuad->GetMapper())
    {
    vtkPoints* points = vtkPoints::New();
    vtkPolyData* pd = vtkPolyData::New();
    vtkCellArray* ca = vtkCellArray::New();
    vtkPolyDataMapper* pdm = vtkPolyDataMapper::New();
    vtkUnsignedCharArray* uca = vtkUnsignedCharArray::New();

    points->SetNumberOfPoints(4);

    double bounds[6];
    this->ImageActor->GetBounds(bounds);

    points->SetPoint(0, bounds[0], bounds[2], 0.0);
    points->SetPoint(1, bounds[1], bounds[2], 0.0);
    points->SetPoint(2, bounds[1], bounds[3], 0.0);
    points->SetPoint(3, bounds[0], bounds[3], 0.0);

    vtkIdType ids[] = { 0, 1, 2, 3 };
    ca->InsertNextCell(4, ids);

    uca->SetNumberOfComponents(4);
    uca->SetName("Colors");

    unsigned char color[4];
    this->FilteringMaskColor.fillArray(color);
    uca->InsertNextTypedTuple(color);

    pd->GetCellData()->SetScalars(uca);
    pd->SetPoints(points);
    pd->SetPolys(ca);
    pdm->SetInputData(pd);

    this->SelectorMaskQuad->SetMapper(pdm);

    points->FastDelete();
    pd->FastDelete();
    ca->FastDelete();
    pdm->FastDelete();
    uca->FastDelete();
    }

  // Foreground color must have non-zero alpha to prevent GL from throwing out
  // the geometry during alpha test.
  vgColor foreground(0.0, 0.0, 0.0, 0.01);

  this->createMaskActor(info, foreground, vgColor());
  this->SelectorMaskProps->AddPart(info.maskActor);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::removeSelectorMask(ContourInfo& info)
{
  this->SelectorMaskProps->RemovePart(info.maskActor);

  if (this->SelectorMaskProps->GetParts()->GetNumberOfItems() == 0)
    this->SelectorMaskQuad->SetMapper(0);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::setAssemblyZRange(vtkAssembly* a, double z1, double z2)
{
  vtkProp3DCollection* props = a->GetParts();
  int numProps = props->GetNumberOfItems();

  double range = z2 - z1;

  int i = 0;
  props->InitTraversal();
  while (vtkProp3D* p = props->GetNextProp3D())
    {
    double z = range * (++i / static_cast<double>((numProps + 1)));
    p->SetPosition(0.0, 0.0, z1 + z);
    }
}

//-----------------------------------------------------------------------------
void vsScenePrivate::updateContourMaskPositions()
{
  // Contour masks should fall in the z interval (-0.1, 0.0), between the
  // video image and the first track or event representation. If contours are
  // not positioned lower than tracks (or events), those tracks will be wrongly
  // occluded, since they render later in the pipeline. For correct rendering,
  // we must also ensure that later-rendered masks have a greater depth value
  // than earlier-rendered ones (of the same type). This prevents z-fighting
  // and prevents blending of the mask color when two mask regions overlap.
  double z = -0.1;
  double w = this->CurrentTransformVtk->GetElement(3, 3);
  double zscale = this->CurrentTransformVtk->GetElement(2, 2);
  if (zscale != 0.0)
    {
    w /= zscale;
    }
  z *= w;
  this->setAssemblyZRange(this->FilterMaskProps, 0.0, z);
  this->setAssemblyZRange(this->SelectorMaskProps, 0.0, z);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::addFilterContour(ContourInfo& info)
{
  vsContour::Type type = info.widget->type();
  vsContourWidget* contourWidget = info.widget.data();
  if (type == vsContour::Filter)
    {
    this->ContourOperatorManager->AddFilter(contourWidget->points());
    this->createFilterMask(info);
    }
  else if (type == vsContour::Selector)
    {
    this->ContourOperatorManager->AddSelector(contourWidget->points());
    this->createSelectorMask(info);
    }
  this->updateContourMaskPositions();
}

//-----------------------------------------------------------------------------
void vsScenePrivate::removeFilterContour(ContourInfo& info)
{
  vsContour::Type type = info.widget->type();
  vsContourWidget* contourWidget = info.widget.data();
  if (type == vsContour::Filter)
    {
    this->removeFilterMask(info);
    this->ContourOperatorManager->RemoveFilter(contourWidget->points());
    }
  else if (type == vsContour::Selector)
    {
    this->removeSelectorMask(info);
    this->ContourOperatorManager->RemoveSelector(contourWidget->points());
    }
}

//-----------------------------------------------------------------------------
void vsScenePrivate::initializeGraph(Graph& graph, const char* labelPrefix)
{
  vtkVgEventTypeRegistry* eventTypeRegistry = this->Core->eventTypeRegistry();

  // Track model
  graph.TrackModel->Initialize();
  graph.TrackModel->SetDisplayAllTracks(true);
  graph.TrackModel->SetContourOperatorManager(this->ContourOperatorManager);

  // Track representations
  graph.TrackRepresentation->UseAutoUpdateOff();
  graph.TrackRepresentation->SetTrackModel(graph.TrackModel);
  graph.TrackRepresentation->SetColorModeToFSO();
  graph.TrackRepresentation->SetTrackFilter(this->TrackFilter);
  graph.TrackRepresentation->SetContourOperatorManager(
    this->ContourOperatorManager);

  graph.TrackHeadRepresentation->SetTrackModel(graph.TrackModel);
  graph.TrackHeadRepresentation->SetColorModeToFSO();
  graph.TrackHeadRepresentation->SetTrackFilter(this->TrackFilter);
  graph.TrackHeadRepresentation->SetContourOperatorManager(
    this->ContourOperatorManager);

  graph.TrackLabelRepresentation->UseAutoUpdateOff();
  graph.TrackLabelRepresentation->SetTrackModel(graph.TrackModel);
  graph.TrackLabelRepresentation->SetTrackFilter(this->TrackFilter);
  if (labelPrefix)
    graph.TrackLabelRepresentation->SetLabelPrefix(labelPrefix);

  // Event model
  graph.EventModel->SetTrackModel(graph.TrackModel);
  graph.EventModel->SetContourOperatorManager(
    this->ContourOperatorManager);

  vtkVgTimeStamp offset;
  offset.SetFrameNumber(30); // 3 seconds at 10 fps
  offset.SetTime(3e6); // 3 seconds
  graph.EventModel->SetEventExpirationOffset(offset);

  // This we do nothing for events whose tracks are used once per trackInfo,
  // which should be true of everything but alerts.  For alerts, this will
  // merge the (possibly) multiple track infos (that use the same track) so
  // that it acts (looks) like single track info.
  graph.EventModel->UseTrackGroupsOn();

  // Event representations
  graph.EventRepresentation->Initialize();
  graph.EventRepresentation->UseAutoUpdateOff();
  graph.EventRepresentation->SetEventModel(graph.EventModel);
  graph.EventRepresentation->SetEventFilter(this->EventFilter);
  graph.EventRepresentation->SetEventTypeRegistry(eventTypeRegistry);

  graph.EventHeadRepresentation->UseAutoUpdateOff();
  graph.EventHeadRepresentation->SetEventModel(graph.EventModel);
  graph.EventHeadRepresentation->SetEventFilter(this->EventFilter);
  graph.EventHeadRepresentation->SetEventTypeRegistry(eventTypeRegistry);

  graph.EventLabelRepresentation->UseAutoUpdateOff();
  graph.EventLabelRepresentation->SetEventModel(graph.EventModel);
  graph.EventLabelRepresentation->SetEventFilter(this->EventFilter);
  graph.EventLabelRepresentation->SetEventTypeRegistry(eventTypeRegistry);
  if (labelPrefix)
    graph.EventLabelRepresentation->SetLabelPrefix(labelPrefix);
  graph.EventLabelRepresentation->SetShowId(false);
  graph.EventLabelRepresentation->SetShowClassifiers(4);

  // Show supporting tracks for events
  graph.EventRepresentation->SetTrackRepresentation(
    graph.TrackRepresentation);
}

//-----------------------------------------------------------------------------
void vsScenePrivate::setupRepresentations(
  Graph& graph,
  float trackHeadWidth, float trackTrailWidth,
  float eventHeadWidth, float eventTrailWidth)
{
  graph.EventRepresentation->SetZOffset(0.1);
  graph.EventRepresentation->SetNormalcyLineWidthScale(0.0f);
  graph.EventRepresentation->SetLineWidth(eventTrailWidth);
  graph.EventHeadRepresentation->SetLineWidth(eventHeadWidth);

  graph.TrackRepresentation->SetActiveTrackLineWidth(trackTrailWidth);
  graph.TrackRepresentation->SetExpiringTrackLineWidth(trackTrailWidth);
  graph.TrackHeadRepresentation->SetLineWidth(trackHeadWidth);
}

//-----------------------------------------------------------------------------
bool vsScenePrivate::updateModels(Graph& graph, bool forceUpdate)
{
  // Events
  QList<vtkVgRepresentationBase*> eventReps;
  QList<vtkProp*> insertEventPropsAfter;
  eventReps << graph.EventRepresentation
            << graph.EventHeadRepresentation
            << graph.EventLabelRepresentation;
  insertEventPropsAfter << this->EventPropsBegin << this->EventPropsBegin << 0;

  // Only notify that the event scene has changed in case of a change to the
  // type registry, event filter, or the event model (not counting normal
  // updates)
  const bool notifyEventSceneUpdated =
    forceUpdate ||
    this->EventFilter->GetMTime() >  graph.EventRepresentationsUpdateTime ||
    graph.EventModel->GetMTime() > graph.EventModel->GetUpdateTime();

  bool updated =
    this->updateModels(graph.EventModel, this->EventFilter,
                       eventReps, insertEventPropsAfter,
                       graph.EventRepresentationsUpdateTime, forceUpdate);
  if (notifyEventSceneUpdated)
    {
    QTE_Q(vsScene);
    emit q->eventSceneUpdated();
    }

  // Tracks
  QList<vtkVgRepresentationBase*> trackReps;
  QList<vtkProp*> insertTrackPropsAfter;
  trackReps << graph.TrackRepresentation
            << graph.TrackHeadRepresentation
            << graph.TrackLabelRepresentation;
  insertTrackPropsAfter << this->TrackPropsBegin << this->TrackPropsBegin << 0;

  // The state of the event representation can affect track display
  forceUpdate = graph.EventRepresentation->GetUpdateTime() >
                graph.TrackRepresentation->GetUpdateTime();

  // Only notify that the track scene has changed in case of a change to the
  // track filter or the model (not counting normal updates)
  const bool notifyTrackSceneUpdated =
    this->TrackFilter->GetMTime() > graph.TrackRepresentationsUpdateTime ||
    graph.TrackModel->GetMTime() > graph.TrackModel->GetUpdateTime();

  updated =
    this->updateModels(graph.TrackModel, this->TrackFilter,
                       trackReps, insertTrackPropsAfter,
                       graph.TrackRepresentationsUpdateTime,
                       updated || forceUpdate) || updated;
  if (notifyTrackSceneUpdated)
    {
    QTE_Q(vsScene);
    emit q->trackSceneUpdated();
    }

  return updated;
}

//-----------------------------------------------------------------------------
bool vsScenePrivate::updateModels(
  vtkVgModelBase* model, vtkObject* filter,
  QList<vtkVgRepresentationBase*> modelReps, QList<vtkProp*> insertAfter,
  vtkTimeStamp& repsUpdateTime, bool forceUpdate)
{
  // Record the old model update time and update the model
  unsigned long preUpdateModelTime = model->GetUpdateTime();
  model->Update(this->CurrentFrameTime,
                &this->CurrentHomographyReferenceTime);

  // If either the model or filter have changed since the last update, then
  // we must perform a full update of all representations.
  if (!forceUpdate)
    {
    forceUpdate =
      (preUpdateModelTime < model->GetUpdateTime() ||
       filter->GetMTime() > repsUpdateTime);
    }

  Q_ASSERT(modelReps.size() == insertAfter.size());

  // See if the model rep Modified and UpdateTime warrant updating the rep
  for (int i = 0, end = modelReps.count(); i < end; ++i)
    {
    vtkVgRepresentationBase* modelRep = modelReps[i];
    if (!modelRep->GetVisible())
      continue;

    if (forceUpdate ||
        modelRep->GetMTime() > modelRep->GetUpdateTime() ||
        modelRep->GetUpdateTime() < model->GetUpdateTime() ||
        modelRep->GetUpdateTime() < model->GetUpdateDataRequestTime())
      {
      modelRep->Update();

      // Add new props
      vtkPropCollection* props;
      props = modelRep->GetNewRenderObjects();
      props->InitTraversal();
      while (vtkProp* prop = props->GetNextProp())
        {
        if (vtkProp* ia = insertAfter[i])
          {
          // Insert prop "by hand" if render order is important. This is
          // a linear operation, but it shouldn't happen that often.
          vtkPropCollection* rendProps = this->Renderer->GetViewProps();
          int insertAt = rendProps->IsItemPresent(ia);
          Q_ASSERT_X(insertAt, __FUNCTION__, "failed assert insertAt != 0;"
                     " placeholder prop not added to renderer");
          rendProps->InsertItem(insertAt - 1, prop);
          prop->AddConsumer(this->Renderer);
          }
        else
          {
          // No placeholder, just append
          this->Renderer->AddViewProp(prop);
          }
        }

      // Remove expired props
      props = modelRep->GetExpiredRenderObjects();
      props->InitTraversal();
      while (vtkProp* prop = props->GetNextProp())
        this->Renderer->RemoveViewProp(prop);

      modelRep->ResetTemporaryRenderObjects();
      }
    }

  // Bump the update time and return true if the model was updated, filters
  // changed, or something else occurred that was important enough force a
  // "full update". This is a signal to the caller that other dependent
  // representations will need updating (e.g. the event tree).
  if (forceUpdate)
    {
    repsUpdateTime.Modified();
    return true;
    }

  // This was an incremental update (e.g. labels only). Further updates of
  // dependent representations should not be necessary.
  return false;
}

//-----------------------------------------------------------------------------
void vsScenePrivate::updateTrackColors()
{
  QTE_Q(vsScene);

  this->TrackColors.clear();
  foreach (vsTrackInfo ti, vsTrackInfo::trackTypes())
    {
    this->TrackColors.insert(ti.type, vgColor(ti.pcolor));

    setTrackTypeColors(this->NormalGraph, ti);
    setTrackTypeColors(this->GroundTruthGraph, ti);

    // TCM_Default doesn't use unclassified color, but that's what we want...
    if (ti.type == vtkVgTrack::Unclassified)
      {
      setDefaultColors(this->NormalGraph, ti.pcolor);
      setDefaultColors(this->GroundTruthGraph, ti.pcolor);
      }
    }

  vsSceneTrackColorHelper* const helper =
    updateTrackColorHelper(this->TrackColorHelper.data());
  setHelperForGraph(this->NormalGraph, helper);
  setHelperForGraph(this->GroundTruthGraph, helper);

  emit q->trackSceneUpdated();
}

//-----------------------------------------------------------------------------
vtkVgTrackInfo vsScenePrivate::trackInfo(vtkIdType trackId)
{
  vtkVgTrackInfo info = this->NormalGraph.TrackModel->GetTrackInfo(trackId);
  if (!info.GetTrack())
    {
    info = this->GroundTruthGraph.TrackModel->GetTrackInfo(trackId);
    }
  return info;
}

//-----------------------------------------------------------------------------
vtkVgEventInfo vsScenePrivate::eventInfo(vtkIdType eventId)
{
  vtkVgEventInfo info = this->NormalGraph.EventModel->GetEventInfo(eventId);
  if (!info.GetEvent())
    {
    info = this->GroundTruthGraph.EventModel->GetEventInfo(eventId);
    }
  return info;
}

//-----------------------------------------------------------------------------
vtkVgTrack* vsScenePrivate::findTrack(vtkIdType trackId)
{
  return this->trackInfo(trackId).GetTrack();
}

//-----------------------------------------------------------------------------
vtkVgEvent* vsScenePrivate::findEvent(vtkIdType eventId)
{
  return this->eventInfo(eventId).GetEvent();
}

//-----------------------------------------------------------------------------
QString vsScenePrivate::buildLocationTextFromDisplay(int x, int y)
{
  if (x < 0 || y < 0)
    {
    // If location is invalid, return empty string
    return QString();
    }
  else if (this->CurrentFrameMetaData.WorldLocation.GCS != -1)
    {
    // Get point in image coordinates
    double pos[3] = { static_cast<double>(x), static_cast<double>(y), 0.0 };
    vtkVgSpaceConversion::DisplayToWorldNormalized(this->Renderer, pos, pos);

    double latitude, longitude;
    vtkSmartPointer<vtkMatrix4x4> imageToLatLonMatrix =
      this->CurrentFrameMetaData.MakeImageToLatLonMatrix();
    if (imageToLatLonMatrix)
      {
      vtkVgApplyHomography(pos, imageToLatLonMatrix, longitude, latitude);
      }
    else
      {
      // Normalize and y-flip the image coordinates
      pos[0] /= (this->ImageActor->GetMaxXBound() + 1);
      pos[1] = 1.0 - (pos[1] / (this->ImageActor->GetMaxYBound() + 1));

      vtkVgVideoFrameCorners& corners =
        this->CurrentFrameMetaData.WorldLocation;
      // Get lat/lon
      latitude =
        blend2d(pos[0], pos[1],
                corners.UpperLeft.Latitude, corners.UpperRight.Latitude,
                corners.LowerLeft.Latitude, corners.LowerRight.Latitude);
      longitude =
        blend2d(pos[0], pos[1],
                corners.UpperLeft.Longitude, corners.UpperRight.Longitude,
                corners.LowerLeft.Longitude, corners.LowerRight.Longitude);
      }

    // Make sure lat-lon is at least in valid range
    latitude = qBound(-90.0, latitude, 90.0);
    longitude = qBound(-180.0, longitude, 180.0);

    // Set clipboard text
    return vgGeodesy::coordString(latitude, longitude);
    }
  return "(invalid)";
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vsScenePrivate::createDummyImage()
{
  // Create a blank image when we don't have a real input
  vtkSmartPointer<vtkImageData> dummy =
    vtkSmartPointer<vtkImageData>::New();
  dummy->SetExtent(0, 9, 0, 9, 0, 0);
  dummy->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  for (int i = 0; i < 10; ++i)
    {
    for (int j = 0; j < 10; ++j)
      {
      dummy->SetScalarComponentFromDouble(i, j, 0, 0, 0);
      }
    }

  return dummy;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vsScenePrivate::createFilterMaskImage(
  const double (&bounds)[6])
{
  int ib[4] =
    {
    vtkMath::Floor(bounds[0]), vtkMath::Ceil(bounds[1]),
    vtkMath::Floor(bounds[2]), vtkMath::Ceil(bounds[3])
    };

  int dim[3] = { ib[1] - ib[0], ib[3] - ib[2], 1 };

  vtkSmartPointer<vtkImageData> data =
    vtkSmartPointer<vtkImageData>::New();
  data->SetExtent(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  data->SetOrigin(ib[0], ib[2], 0);
  data->AllocateScalars(VTK_FLOAT, 1);

  // Initialize to 0.0f (CAUTION: relies on 0.0f being binary 0)
  size_t k = dim[0] * dim[1] * dim[2];
  memset(data->GetScalarPointer(), 0, k * sizeof(float));

  return data;
}

//-----------------------------------------------------------------------------
void vsScenePrivate::setRepresentationTransforms(
  Graph& graph, vtkMatrix4x4* xf)
{
  graph.TrackRepresentation->SetRepresentationMatrix(xf);
  graph.TrackHeadRepresentation->SetRepresentationMatrix(xf);
  graph.TrackLabelRepresentation->SetRepresentationMatrix(xf);

  graph.EventRepresentation->SetRepresentationMatrix(xf);
  graph.EventHeadRepresentation->SetRepresentationMatrix(xf);
  graph.EventLabelRepresentation->SetRepresentationMatrix(xf);
}

//END vsScenePrivate
