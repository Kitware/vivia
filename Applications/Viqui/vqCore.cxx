/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqCore.h"

// Qt includes
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QTemporaryFile>
#include <QTimer>
#include <QTimerEvent>

// QtExtensions includes
#include <qtMap.h>
#include <qtStatusManager.h>
#include <qtStlUtil.h>

// VTK includes
#include <vtkActor.h>
#include <vtkAreaPicker.h>
#include <vtkCellArray.h>
#include <vtkContourWidget.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyle.h>
#include <vtkLookupTable.h>
#include <vtkLinearContourLineInterpolator.h>
#include <vtkMath.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3D.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderedAreaPicker.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkTimerLog.h>

// VISGUI includes
#include <vtkVgSelectionWidget.h> // will be moved to VTK in future
#include <vtkVgContourOperatorManager.h>
#include <vtkVgEvent.h>
#include <vtkVgEventBase.h>
#include <vtkVgInteractorStyleRubberBand2D.h>
#include <vtkVgSelectionListRepresentation.h>
#include <vtkVgSpaceConversion.h>
#include <vtkVgTimeStamp.h>
#include <vtkVgTrack.h>
#include <vtkVgUtil.h>

#include <vtkVgEventModel.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgTrackModel.h>
#include <vtkVgTrackRepresentation.h>
#include <vtkVgTripWireManager.h>
#include <vtkVgVideoModel0.h>
#include <vtkVgVideoRepresentation0.h>

#include <vtkVgFindNode.h>
#include <vtkVgFindNodeVisitor.h>
#include <vtkVgGeode.h>
#include <vtkVgGroupNode.h>
#include <vtkVgTerrain.h>
#include <vtkVgTrackNode.h>
#include <vtkVgTransformNode.h>
#include <vtkVgVideoNode.h>
#include <vtkVgVideoViewer.h>
#include <vtkVgViewerBase.h>

#include <vtkVgQtUtil.h>

// VgCommon includes
#include <vgCheckArg.h>

#include <vgGeoUtil.h>

// VgVideo includes
#include <vgKwaArchive.h>
#include <vgKwaFrameMetadata.h>
#include <vgKwaVideoClip.h>

// VV includes
#include <vvQueryResult.h>

#include <vvKmlLine.h>
#include <vvKmlWriter.h>
#include <vvKstWriter.h>
#include <vvUtil.h>

#include <vvReportWriter.h>

#include <vtkVgTerrainSource.h>

// Viqui includes
#include "Backends/vqExporterFactory.h"
#include "vqArchiveVideoSource.h"
#include "vqContour.h"
#include "vqDebug.h"
#include "vqQueryDialog.h"
#include "vqQueryParser.h"
#include "vqScopedOverrideCursor.h"
#include "vqSettings.h"
#include "vqTrackingClipViewer.h"
#include "vqVideoNodeVisitor.h"
#include "vtkVgCoordinateTransform.h"
#include "vtkVQTrackingClip.h"
#include "vtkVQInteractionCallback.h"
#include "vtkVQBlastLayoutNode.h"
#include "vtkVQInteractionCallback.h"

// C/C++ includes
#include <algorithm>
#include <cassert>
#include <exception>
#include <map>
#include <sstream>
#include <vector>
#include <string>

namespace // anonymous
{

double SelectedTrackColor[] = { 1.0, 0.08, 0.58 };

//-----------------------------------------------------------------------------
bool CompareByScore(vtkVgVideoNode* a, vtkVgVideoNode* b)
{
  return (a->GetRelevancyScore() == b->GetRelevancyScore())
         ? a->GetInstanceId() < b->GetInstanceId()
         : a->GetRelevancyScore() > b->GetRelevancyScore();
}

//-----------------------------------------------------------------------------
bool CompareByRank(vtkVgVideoNode* a, vtkVgVideoNode* b)
{
  return (a->GetRank() == b->GetRank())
         ? CompareByScore(a, b)
         : a->GetRank() < b->GetRank();
}

//-----------------------------------------------------------------------------
class RenderWidgetEventFilter : public QObject
{
public:
  RenderWidgetEventFilter(QObject* parent, vqCore* core) :
    QObject(parent), Core(core) {}

  virtual bool eventFilter(QObject* /*sender*/, QEvent* e)
    {
    if (e->type() == QEvent::Resize)
      {
      // Update terrain source in order to update context tile visible extents,
      // and repaint; must invoke queued so that the resize completes first!
      QMetaObject::invokeMethod(this->Core, "updateSources",
                                Qt::QueuedConnection);
      }
    return false;
    }

private:
  vqCore* Core;
};

//-----------------------------------------------------------------------------
vvQueryResult stripResult(vvQueryResult result)
{
  const size_t i = vvUtil::findMergedRegions(result.Descriptors);
  const size_t k = result.Descriptors.size();
  if (i < k)
    {
    vvDescriptor mergedRegionsDescriptor = result.Descriptors[i];

    // Generate set of used tracks
    QSet<vvTrackId> tracks;
    for (size_t n = 0; n < k; ++n)
      {
      const vvDescriptor& descriptor = result.Descriptors[n];
      for (size_t tn = 0, tk = descriptor.TrackIds.size(); tn < tk; ++tn)
        {
        tracks.insert(descriptor.TrackIds[tn]);
        }
      }

    // Assign track ID's to merged region descriptor, so that they aren't
    // dropped (okay to clear its tracks first, as we would have included its
    // tracks, if any, when building the set)
    mergedRegionsDescriptor.TrackIds.clear();
    foreach (const vvTrackId& tid, tracks)
      {
      mergedRegionsDescriptor.TrackIds.push_back(tid);
      }

    // Replace the result descriptor set with just the merged descriptor
    result.Descriptors.clear();
    result.Descriptors.push_back(mergedRegionsDescriptor);
    }

  return result;
}

//-----------------------------------------------------------------------------
bool computeImageToLatLon(
  vtkSmartPointer<vtkMatrix4x4>& out,
  const vgKwaFrameMetadata& frameMetaData)
{
  CHECK_ARG(frameMetaData.worldCornerPoints().GCS != -1, false);

  vtkVgCoordinateTransform::SmartPtr imageToLatLon(
    vtkVgCoordinateTransform::SmartPtr::New());

  imageToLatLon->SetToPoints(
    frameMetaData.worldCornerPoints().UpperLeft.Easting,
    frameMetaData.worldCornerPoints().UpperLeft.Northing,
    frameMetaData.worldCornerPoints().UpperRight.Easting,
    frameMetaData.worldCornerPoints().UpperRight.Northing,
    frameMetaData.worldCornerPoints().LowerRight.Easting,
    frameMetaData.worldCornerPoints().LowerRight.Northing,
    frameMetaData.worldCornerPoints().LowerLeft.Easting,
    frameMetaData.worldCornerPoints().LowerLeft.Northing);

  double iw = frameMetaData.imageSize().width();
  double ih = frameMetaData.imageSize().height();
  imageToLatLon->SetFromPoints(0.0,  ih - 1.0,
                               iw - 1.0,  ih - 1.0,
                               iw - 1.0,       0.0,
                               0.0,       0.0);

  out = imageToLatLon->GetHomographyMatrix();
  return !!out;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vqCore::vqCore()
  : QObject(), SavePersistentQuery(false),
    InteractionStatusSource(this), QuerySessionDummyObject(),
    QuerySessionStatusSource(&this->QuerySessionDummyObject)
{
  this->StatusManager           = new qtStatusManager(this);
  this->StatusManager->setDebugArea(vqdStatusManager);

  this->QuerySessionStatusSource.setName(this);

  // Create viewer.
  this->ContextViewer = vtkSmartPointer<vtkVgVideoViewer>::New();
  this->ContextViewer->SetSceneRoot(this->createScene());

  this->ContextInteractorStyle  = vtkSmartPointer<vtkVgInteractorStyleRubberBand2D>::New();

  this->TerrainSource           = 0;

  this->ContextVideoRoot        = 0;

  this->TimerLog = vtkSmartPointer<vtkTimerLog>::New();
  this->ApplicationStartTime = this->TimerLog->GetUniversalTime();

  this->ApplicationTimeStamp.SetTime(0.0);

  this->ActiveQueryParser       = 0;

  this->LastEventViewScale      = 0.0;
  this->LastMarkerViewScale     = 0.0;

  this->NodeColorMappingMode   = ColorByAbsoluteScore;
  this->NodeColorShadingMode   = qtGradient::InterpolateDiscrete;

  this->ItemColorLUT            = vtkSmartPointer<vtkLookupTable>::New();

  this->ViewScaleFactor         = 3.5;

  // Set callbacks.
  vtkSmartPointer<vtkVQInteractionCallback> interactionCallback
  (vtkSmartPointer<vtkVQInteractionCallback>::New());
  interactionCallback->SetCore(this);

  this->ContextInteractorStyle->AddObserver(
    vtkCommand::InteractionEvent, interactionCallback);
  this->ContextInteractorStyle->AddObserver(
    vtkVgInteractorStyleRubberBand2D::ZoomCompleteEvent, interactionCallback);

  vtkMath::UninitializeBounds(this->TerrainBounds);

  this->CachedLayoutNode = 0;

  this->QueryResultNodesDirty = false;
  this->ScoringRequestNodesDirty = false;
  this->GroundTruthNodesDirty = false;

  this->Contour = 0;
  this->QueryRegionContour = 0;
  this->QueryRegionVisible = true;
  this->DrawingContour = false;

  this->SkipInitialDisplayCounter = 0;

  // Approximately 15 FPS.
  this->UpdateIntervalMSec = 60;

  this->UpdateTimer = 0;

  // Defaults unless loaded from file
  this->SpatialFilterActive = false;
  this->QueryResultSelector = vtkSmartPointer<vtkVgContourOperatorManager>::New();
  this->QueryTripWireManager = vtkSmartPointer<vtkVgTripWireManager>::New();
  this->QueryTripWireManager->SetTripWireId(-3000);
  this->QueryTripWireManager->SetEnteringRegionId(-3001);
  this->QueryTripWireManager->SetExitingRegionId(-3002);

  this->ReceivingGroundTruthResults = false;
  this->GroundTruthEventType = -1; // show all types

  this->RenderPending = false;

  this->ContinuousRender = false;

  this->TrackNode = vtkSmartPointer<vtkVgTrackNode>::New();
  this->ContextSceneRoot->AddChild(this->TrackNode);

  this->ShowVideoClipsOnContext = true;
  this->ShowVideoOutlinesOnContext = true;
  this->ShowTracksOnContext = false;

  this->ProcessingDatabaseQuery = false;

  this->ContextTracksSelector
    = vtkSmartPointer<vtkVgContourOperatorManager>::New();

  this->setupTracksOnContext();
}

//-----------------------------------------------------------------------------
vqCore::~vqCore()
{
  this->unregisterAllNodes();

  delete this->ActiveQueryParser;
  this->ActiveQueryParser = 0;

  delete this->Contour;
  this->Contour = 0;

  delete this->QueryRegionContour;
  this->QueryRegionContour = 0;
}

//-----------------------------------------------------------------------------
void vqCore::setupUi(QVTKWidget* renderWidget)
{
  if (renderWidget)
    {
    this->RenderWidget = renderWidget;
    this->ContextViewer->SetRenderWindowInteractor(
      renderWidget->GetInteractor());

    renderWidget->SetRenderWindow(this->ContextViewer->GetRenderWindow());
    renderWidget->GetInteractor()->SetInteractorStyle(
      this->ContextInteractorStyle);

    this->ContextInteractorStyle->SetRenderer(
      this->ContextViewer->GetSceneRenderer());
    this->ContextInteractorStyle->RubberBandSelectionWithCtrlKeyOn();

    renderWidget->installEventFilter(
      new RenderWidgetEventFilter(renderWidget, this));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkVgInteractorStyleRubberBand2D::LeftClickEvent,
               this, SLOT(onLeftClick()));

    vtkConnect(renderWidget->GetInteractor(),
               vtkCommand::RightButtonPressEvent,
               this, SLOT(onRightButtonPress()));

    vtkConnect(renderWidget->GetInteractor(),
               vtkCommand::RightButtonReleaseEvent,
               this, SLOT(onRightButtonRelease()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkCommand::MouseWheelForwardEvent,
               this, SLOT(onMouseWheelForward()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkCommand::MouseWheelBackwardEvent,
               this, SLOT(onMouseWheelBackward()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkVgInteractorStyleRubberBand2D::KeyPressEvent_P,
               this, SLOT(onPKeyPress()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkVgInteractorStyleRubberBand2D::KeyPressEvent_R,
               this, SLOT(resetView()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkVgInteractorStyleRubberBand2D::KeyPressEvent_A,
               this, SLOT(resetViewToQueryResults()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkCommand::InteractionEvent,
               this, SLOT(updateLayoutStackBarLocations()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkVgInteractorStyleRubberBand2D::ZoomCompleteEvent,
               this, SLOT(updateLayoutStackBarLocations()));

    vtkConnect(renderWidget->GetInteractor()->GetInteractorStyle(),
               vtkVgInteractorStyleRubberBand2D::SelectionCompleteEvent,
               this, SLOT(selectItemsOnContext()));
    }
}

//-----------------------------------------------------------------------------
void vqCore::registerStatusWidget(QLabel* widget)
{
  this->StatusManager->addStatusLabel(widget);
}

//-----------------------------------------------------------------------------
void vqCore::registerStatusWidget(QProgressBar* widget)
{
  this->StatusManager->addProgressBar(widget);
}

//-----------------------------------------------------------------------------
vtkVgViewerBase* vqCore::getContextViewer()
{
  return this->ContextViewer.GetPointer();
}

//-----------------------------------------------------------------------------
vtkVgInteractorStyleRubberBand2D* vqCore::getContextInteractorStyle()
{
  return this->ContextInteractorStyle;
}

//-----------------------------------------------------------------------------
int vqCore::resultCount() const
{
  return this->QueryResults.count();
}

//-----------------------------------------------------------------------------
QList<vtkVgVideoNode*> vqCore::queryResultNodes()
{
  if (this->QueryResultNodesDirty)
    {
    this->QueryResultNodeList = this->QueryResultNodes.values();

    qSort(this->QueryResultNodeList.begin(),
          this->QueryResultNodeList.end(),
          &CompareByRank);

    this->QueryResultNodesDirty = false;
    }

  return this->QueryResultNodeList;
}

//-----------------------------------------------------------------------------
QList<vtkVgVideoNode*> vqCore::scoringRequestNodes()
{
  if (this->ScoringRequestNodesDirty || this->QueryResultNodesDirty)
    {
    this->ScoringRequestNodeList = this->ScoringRequestNodes.values();

    qSort(this->ScoringRequestNodeList.begin(),
          this->ScoringRequestNodeList.end(),
          &CompareByRank);

    this->ScoringRequestNodesDirty = false;
    }

  return this->ScoringRequestNodeList;
}

//-----------------------------------------------------------------------------
QList<vtkVgVideoNode*> vqCore::groundTruthNodes()
{
  if (this->GroundTruthNodesDirty)
    {
    this->GroundTruthNodeList = this->GroundTruthNodes.values();

    qSort(this->GroundTruthNodeList.begin(),
          this->GroundTruthNodeList.end(),
          &CompareByScore);

    ResultRank rank = 1;
    foreach (vtkVgVideoNode* node, this->GroundTruthNodeList)
      {
      node->SetRank(rank++);
      }

    this->GroundTruthNodesDirty = false;
    }

  return this->GroundTruthNodeList;
}

//-----------------------------------------------------------------------------
vtkLookupTable* vqCore::getLookupTable()
{
  return this->ItemColorLUT;
}

//-----------------------------------------------------------------------------
void vqCore::resetQueryResults(bool clearResults, bool clearDisplayResults)
{
  if (clearResults)
    {
    // Clear out entire result list; usually means we are starting a new query.
    this->QueryResults.clear();
    this->QueryScoreMap.clear();
    this->ScoringRequests.clear();
    emit this->queryResultsAvailabilityChanged(false);
    }
  else if (clearDisplayResults)
    {
    // Clear out display results but not the full result set; usually means we
    // are going to display a different subset due to IQR or a change in the
    // display offset.
    this->QueryScoreMap.clear();
    }

  // Record results that have been starred.
  this->StarResults.clear();
  if (!clearResults)
    {
    QHash<ResultId, vtkVgVideoNode*>::iterator itr;
    QHash<ResultId, vtkVgVideoNode*>::iterator end;
    for (itr = this->QueryResultNodes.begin(),
         end = this->QueryResultNodes.end(); itr != end; ++itr)
      {
      if (itr.value()->GetIsStarResult())
        {
        this->StarResults.insert(itr.key());
        }
      }
    }

  // Clear out scene elements.
  this->unregisterAllNodes();

  this->QueryResultNodes.clear();
  this->ScoringRequestNodes.clear();
  this->GroundTruthNodes.clear();
  this->QueryResultNodeList.clear();
  this->ScoringRequestNodeList.clear();
  this->GroundTruthNodeList.clear();

  this->QueryResultNodesDirty = true;
  this->ScoringRequestNodesDirty = true;
  this->GroundTruthNodesDirty = true;

  // Remove the last result.
  if (this->ContextVideoRoot)
    {
    this->ContextSceneRoot->RemoveChild(this->ContextVideoRoot);
    }

  this->ContextVideoRoot = vtkVgTransformNode::SmartPtr::New();
  this->ContextVideoRoot->SetName("ContextVideoRoot");

  // \NOTE: Setting the matrix at the root of video context.
  if (this->ContextVideoRoot)
    {
    this->ContextVideoRoot->SetMatrix(
      this->TerrainSource->GetCoordinateTransformMatrix());
    }

  this->ContextSceneRoot->AddChild(this->ContextVideoRoot);

  // Clear the track model
  this->TrackNode->GetTrackModel()->Initialize();

  emit this->displayResultsReset();
}

//-----------------------------------------------------------------------------
void vqCore::registerStatusSource(qtStatusNotifier* source)
{
  // \TODO remove this method when video query can use its own parser
  source->addReceiver(this->StatusManager);
}

//-----------------------------------------------------------------------------
void vqCore::connectParser(vqQueryParser* parser)
{
  connect(parser, SIGNAL(resultAvailable(vvQueryResult, bool)),
          this, SLOT(acceptQueryResult(vvQueryResult, bool)));
  connect(parser, SIGNAL(resultSetComplete(bool)),
          this, SLOT(queryFinished(bool)));

  parser->addReceiver(this->StatusManager);

  connect(parser, SIGNAL(error(qtStatusSource, QString)),
          this->StatusManager, SLOT(setStatusText(qtStatusSource, QString)));
}

//-----------------------------------------------------------------------------
bool vqCore::canIssueQuery()
{
  // Context must be loaded before we can issue queries
  if (!this->TerrainSource)
    {
    QMessageBox::warning(0, "Viqui", "Load context first");
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> vqCore::getLatLonToContextMatrix() const
{
  return this->TerrainSource->GetCoordinateTransformMatrix();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> vqCore::getContextToLatLonMatrix() const
{
  vtkSmartPointer<vtkMatrix4x4> invertMat = vtkSmartPointer<vtkMatrix4x4>::New();
  invertMat->DeepCopy(this->TerrainSource->GetCoordinateTransformMatrix());
  invertMat->Invert();

  return invertMat;
}

//-----------------------------------------------------------------------------
void vqCore::getCurrentPointerPosition(int& x, int& y)
{
  int* pointerPosition = this->ContextInteractorStyle->GetEndPosition();
  x = pointerPosition[0];
  y = pointerPosition[1];
}

//-----------------------------------------------------------------------------
void vqCore::transformDisplayToLatLon(int x, int y, double& lat, double& lon) const
{
  double pt[3] = { static_cast<double>(x), static_cast<double>(y), 0.0 };

  vtkVgSpaceConversion::DisplayToWorldNormalized(
    this->ContextViewer->GetSceneRenderer(), pt, pt);

  vtkSmartPointer<vtkMatrix4x4> contextToLatLon =
    this->getContextToLatLonMatrix();

  vtkVgApplyHomography(pt, contextToLatLon, lon, lat);
}

//-----------------------------------------------------------------------------
void vqCore::transformContextToLatLon(
  double x, double y, double& lat, double& lon) const
{
  vtkSmartPointer<vtkMatrix4x4> contextToLatLon =
    this->getContextToLatLonMatrix();

  vtkVgApplyHomography(x, y, contextToLatLon, lon, lat);
}

//-----------------------------------------------------------------------------
bool vqCore::startQuerySession(bool formulate)
{
  if (!this->canIssueQuery())
    return false;

  // Terminate any currently-active query
  this->endQuerySession(!formulate);

  this->ActiveQueryParser = new vqQueryParser(this->QueryServer);

  connectParser(this->ActiveQueryParser);

  // TODO: Formulate should use a stand-alone parser when we can trust all of
  //       out back-ends to support simultaneous requests, at which point these
  //       connections (and the formulationComplete signal itself in vqCore,
  //       and our parameter) should go away
  if (formulate)
    {
    connect(this->ActiveQueryParser,
            SIGNAL(formulationComplete(QList<vvDescriptor>, QList<vvTrack>)),
            this,
            SIGNAL(formulationComplete(QList<vvDescriptor>, QList<vvTrack>)));
    connect(this->ActiveQueryParser,
            SIGNAL(formulationComplete(QList<vvDescriptor>, QList<vvTrack>)),
            this, SLOT(formulationFinished()));
    connect(this->ActiveQueryParser, SIGNAL(error(qtStatusSource, QString)),
            this, SIGNAL(formulationFailed()));
    connect(this->ActiveQueryParser, SIGNAL(error(qtStatusSource, QString)),
            this, SLOT(formulationFinished()));
    }
  else
    {
    connect(this->ActiveQueryParser, SIGNAL(error(qtStatusSource, QString)),
            this, SLOT(transferQueryStatus()));
    connect(this->ActiveQueryParser, SIGNAL(finished()),
            this, SLOT(releaseQuery()));
    }

  return true;
}

//-----------------------------------------------------------------------------
void vqCore::endQuerySession(bool clearResults)
{
  if (this->ActiveQueryParser)
    {
    this->StatusManager->setStatusText(this->QuerySessionStatusSource,
                                       "Shutting down query session...");
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    delete this->ActiveQueryParser;
    }

  this->ReceivingGroundTruthResults = false;

  // clear out any tracks in our track model
  this->TrackNode->GetTrackModel()->Initialize();

  // \TODO remove parameter (always true) when formulate doesn't use
  //       ActiveQueryParser
  if (clearResults)
    {
    this->resetQueryResults(true);
    }
  this->SavedQueryPlan.clear();
  emit this->queryPlanAvailabilityChanged(false);

  if (this->ActiveQueryParser)
    {
    QApplication::restoreOverrideCursor();
    }

  this->StatusManager->setStatusText(this->QuerySessionStatusSource);
  this->ActiveQueryParser = 0;
}

//-----------------------------------------------------------------------------
bool vqCore::loadQuery(QUrl query, LoadQueryFlags flags)
{
  // \NOTE: We only support queries from a file at this moment.
  if (query.scheme().toLower() != "file")
    {
    QMessageBox::critical(0, "Viqui",
                          "Must load queries from local files for now");
    return false;
    }

  if (!this->canIssueQuery())
    return false;

  bool isGroundTruth = flags.testFlag(vqCore::GroundTruth);
  this->SavePersistentQuery = flags.testFlag(vqCore::SavePersistent);

  // Stop any existing session and clear out any old results
  this->endQuerySession(!isGroundTruth || !this->ContextVideoRoot);

  this->ReceivingGroundTruthResults = isGroundTruth;

  // File loading doesn't involve a session or need a query server, so do it
  // from a stand-alone parser
  vqQueryParser parser;
  this->connectParser(&parser);
  connect(&parser, SIGNAL(planAvailable(vvQueryInstance)),
          this, SLOT(processQuery(vvQueryInstance)));
  connect(&parser, SIGNAL(finished()),
          this, SLOT(finalizeQueryLoading()));

  bool result = parser.loadQuery(query);
  this->StatusManager->transferOwnership(&parser,
                                         this->QuerySessionStatusSource);
  return result;
}

//-----------------------------------------------------------------------------
void vqCore::finalizeQueryLoading()
{
  // Done (for better or worse) trying to load a query; clear flag to save the
  // next query as the edit dialog persistent query so we don't stuff some
  // random other query in there
  this->SavePersistentQuery = false;
}

//-----------------------------------------------------------------------------
bool vqCore::formulateQuery(
  vvProcessingRequest request, bool bypassCache/*= false*/,
  qtStatusManager* statusTarget/*= 0*/)
{
  if (!this->startQuerySession(true))
    return false;

  if (statusTarget)
    this->ActiveQueryParser->addReceiver(statusTarget);

  return this->ActiveQueryParser->formulateQuery(request, bypassCache);
}

//-----------------------------------------------------------------------------
bool vqCore::processQuery(vvQueryInstance mutableQuery)
{
  // Query is passed by-value because this is a slot, but we shouldn't modify
  // it; alias it here as a const reference so the compiler will enforce that
  const vvQueryInstance& query = mutableQuery;
  if (!(query.isValid() && this->startQuerySession(false)))
    return false;

  if (this->SavePersistentQuery)
    {
    vqQueryDialog::savePersistentQuery(mutableQuery);
    this->SavePersistentQuery = false;
    }

  this->SpatialFilterActive = false;
  this->QueryResultSelector->RemoveAllSelectors();
  this->QueryTripWireManager->RemoveAllTripWires();

  // remove any old contours
  delete this->QueryRegionContour;
  delete this->Contour;
  this->QueryRegionContour = 0;
  this->Contour = 0;

  if (query.constAbstractQuery()->SpatialLimit.Coordinate.size() > 0)
    {
    this->SpatialFilterActive = true;

    // create a contour based on the query region
    this->QueryRegionContour =
      this->createStaticContour(query.constAbstractQuery()->SpatialLimit, true);

    // filter
    this->QueryResultSelector->AddSelector(
      this->QueryRegionContour->GetPolyData()->GetPoints());

    // trip wire
    bool useTripwire;
    switch (query.constAbstractQuery()->SpatialFilter)
      {
      case vvDatabaseQuery::Intersects:
        this->FilterOnEnterTripEvents = true;
        this->FilterOnExitTripEvents = true;
        useTripwire = true;
        break;
      case vvDatabaseQuery::IntersectsInbound:
        this->FilterOnEnterTripEvents = true;
        this->FilterOnExitTripEvents = false;
        useTripwire = true;
        break;
      case vvDatabaseQuery::IntersectsOutbound:
        this->FilterOnEnterTripEvents = false;
        this->FilterOnExitTripEvents = true;
        useTripwire = true;
        break;
      default:
        this->FilterOnEnterTripEvents = false;
        this->FilterOnExitTripEvents = false;
        useTripwire = false;
        break;
      }
    if (useTripwire)
      {
      this->QueryTripWireManager->AddTripWire(
        this->QueryRegionContour->GetPolyData()->GetPoints(), true);
      }
    }

  // Note:  This is temporary code to facilitate doing Refine in presence of an
  // IQR model in the submitted query, without teasing the user with the
  // initial result set; see commit for necessary code to remove when the
  // the system supports IQR modified results on initial query
  if (query.constSimilarityQuery() &&
      query.constSimilarityQuery()->IqrModel.size())
    {
    this->SkipInitialDisplayCounter = 2;
    }
  else
    {
    this->SkipInitialDisplayCounter = 0;
    }

  if (!this->ActiveQueryParser->processQuery(query))
    {
    delete this->ActiveQueryParser;
    this->ActiveQueryParser = 0;
    return false;
    }

  // Remember query plan so we can (potentially) save it later
  this->SavedQueryPlan = query;
  emit this->queryPlanAvailabilityChanged(true);

  emit this->processingQuery(query);
  return true;
}

//-----------------------------------------------------------------------------
bool vqCore::processDatabaseVideoQuery(vvQueryInstance queryInstance)
{
  if (!this->startQuerySession(true))
    {
    return false;
    }

  if (!this->ActiveQueryParser->processQuery(queryInstance))
    {
    delete this->ActiveQueryParser;
    this->ActiveQueryParser = 0;
    return false;
    }

  this->ProcessingDatabaseQuery = true;

  return true;
}

//-----------------------------------------------------------------------------
bool vqCore::requestRefinement(int resultsToScore)
{
  if (!this->ActiveQueryParser)
    {
    QMessageBox::warning(0, "Viqui", "There is no active query session");
    return false;
    }

  return this->ActiveQueryParser->requestScoring(resultsToScore);
}

//-----------------------------------------------------------------------------
bool vqCore::refineQuery()
{
  if (!this->ActiveQueryParser)
    {
    QMessageBox::warning(0, "Viqui", "There is no active query session");
    return false;
    }

  vvIqr::ScoringClassifiers feedback;
  foreach (ResultId iid, this->ScoringRequestNodes.keys())
    {
    feedback.insert(iid, this->QueryResults[iid].UserScore);
    }

  foreach (ResultId iid, this->QueryResultNodes.keys())
    {
    vvIqr::Classification status = this->QueryResults[iid].UserScore;
    if (status == vvIqr::PositiveExample || status == vvIqr::NegativeExample)
      {
      feedback.insert(iid, status);
      }
    }

  if (feedback.isEmpty())
    {
    QMessageBox::warning(0, "Viqui", "There is no feedback to provide");
    return false;
    }

  this->resetQueryResults(false);

  this->ActiveQueryParser->refineQuery(feedback);
  return true;
}

//-----------------------------------------------------------------------------
void vqCore::acceptQueryResult(vvQueryResult queryResult, bool requestScoring)
{

  if (this->ProcessingDatabaseQuery)
    {
    this->CacheDatabaseQueryResults(queryResult);
    return;
    }

  if (this->ReceivingGroundTruthResults)
    {
    this->addGroundTruthResult(queryResult);
    return;
    }

  if (!requestScoring)
    {
    // If this is part of the query results, just add it (or update the score,
    // if this is a refinement iteration)
    this->addQueryResult(queryResult);
    }
  else
    {
    ResultId iid = queryResult.InstanceId;
    // Requesting refinement on a result should not change the score...
    if (!this->QueryResults.contains(iid))
      {
      // Haven't seen this result? Not expected to happen, but just in case...
      this->addQueryResult(queryResult);
      }
    else // do, however, update the preference score
      {
      this->QueryResults[iid].PreferenceScore = queryResult.PreferenceScore;
      // Since this was a main query result, may have created a QueryResultNode
      // that we need to update
      if (this->QueryResultNodes.contains(iid))
        {
        this->QueryResultNodes[iid]->
        SetPreferenceScore(queryResult.PreferenceScore);
        }
      }

    // Now create the node if it doesn't already exist
    this->ScoringRequests.insert(iid);
    this->createScoringRequestNode(iid);
    }
}

//-----------------------------------------------------------------------------
void vqCore::formulationFinished()
{
  // Release active query, but keep last status message
  this->transferQueryStatus();
  this->releaseQuery();
}

//-----------------------------------------------------------------------------
void vqCore::transferQueryStatus()
{
  this->StatusManager->transferOwnership(this->ActiveQueryParser,
                                         this->QuerySessionStatusSource);
}

//-----------------------------------------------------------------------------
void vqCore::releaseQuery()
{
  if (this->ActiveQueryParser)
    this->ActiveQueryParser->deleteLater();
  this->ActiveQueryParser = 0;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgGroupNode> vqCore::createScene()
{
  this->ContextSceneRoot  =  vtkVgTransformNode::SmartPtr::New();
  this->ContextSceneRoot->SetName("ContextSceneRoot");
  return this->ContextSceneRoot;
}

//-----------------------------------------------------------------------------
void vqCore::start()
{
  this->reloadColorGradient();
  this->ContextViewer->Run();

  if (!this->UpdateTimer)
    {
    this->UpdateTimer = new QTimer(this);
    connect(this->UpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
    }
}

//-----------------------------------------------------------------------------
void vqCore::update()
{
  // Microseconds is what we are using
  this->ApplicationTimeStamp.SetTime
  (1e6 * (this->TimerLog->GetUniversalTime() - this->ApplicationStartTime));

  this->ContextViewer->Frame(this->ApplicationTimeStamp);
  this->postRender();
}

//-----------------------------------------------------------------------------
void vqCore::postRender()
{
  if (!this->RenderPending)
    {
    // Render at the end of the event loop, which is when everything should
    // be done updating and we will be ready to render of the scene.
    this->RenderPending = true;
    QMetaObject::invokeMethod(this, "forceRender", Qt::QueuedConnection);
    }
}

//-----------------------------------------------------------------------------
void vqCore::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == this->ClickTimer.timerId())
    {
    this->ClickTimer.stop();
    }
}

//-----------------------------------------------------------------------------
void vqCore::setupTracksOnContext()
{
  assert(this->TrackNode);

  vtkVgTrackRepresentation::SmartPtr trackRepresentation(
    vtkVgTrackRepresentation::SmartPtr::New());
  vtkVgTrackModel::SmartPtr trackModel(
    vtkVgTrackModel::SmartPtr::New());
  trackModel->ShowTracksBeforeStartOn();
  trackModel->ShowTracksAfterExpirationOn();

  // Initially not visible
  this->TrackNode->SetVisible(true);
  trackRepresentation->SetVisible(false);
  trackRepresentation->SetTrackModel(trackModel);

  this->TrackNode->SetTrackModel(trackModel);
  this->TrackNode->SetTrackRepresentation(trackRepresentation);
}

//-----------------------------------------------------------------------------
void vqCore::selectClipsOnContext(
  int* startScreenPosition, int* endScreenPosition)
{
  if (!startScreenPosition || !endScreenPosition || !this->ContextVideoRoot)
    {
    return;
    }

  vtkRenderer* ren = this->ContextViewer->GetSceneRenderer();
  vtkSmartPointer<vtkRenderedAreaPicker> area =
    vtkSmartPointer<vtkRenderedAreaPicker>::New();
  int picked = area->AreaPick(
    startScreenPosition[0], startScreenPosition[1],
    endScreenPosition[0],   endScreenPosition[1], ren);

  if (picked != 1)
    {
    return;
    }

  vtkProp3DCollection* propCollection = area->GetProp3Ds();
  this->ContextViewer->GetRenderWindowInteractor()->SetPicker(area);

  propCollection->InitTraversal();
  vtkProp3D* prop = propCollection->GetNextProp3D();

  vtkVgFindNodeVisitor findNodeVisitor;
  QList<vtkVgNodeBase*> selectedNodes;
  while (prop)
    {
    findNodeVisitor.SetUsingProp3D(prop);

    this->ContextVideoRoot->Accept(findNodeVisitor);

    vtkVgNodeBase* node = findNodeVisitor.GetNode();
    vtkVgVideoNode* videoNode = dynamic_cast<vtkVgVideoNode*>(node);

    // If this is a video node and not already existed in list.
    if (videoNode && !selectedNodes.contains(node))
      {
      selectedNodes.push_back(node);
      }

    prop = propCollection->GetNextProp3D();
    }

  this->selectNodes(selectedNodes);
}

//-----------------------------------------------------------------------------
void vqCore::selectTracksOnContext(
  int* startScreenPosition, int* endScreenPosition)
{
  if (!startScreenPosition || !endScreenPosition)
    {
    return;
    }

  if (!this->TrackNode)
    {
    return;
    }

  double startPoint[3] =
    {
    static_cast<double>(startScreenPosition[0]),
    static_cast<double>(startScreenPosition[1]),
    static_cast<double>(1)
    };

  double endPoint[3] =
    {
    static_cast<double>(endScreenPosition[0]),
    static_cast<double>(endScreenPosition[1]),
    static_cast<double>(1)
    };

  vtkVgSpaceConversion::DisplayToWorldNormalized(
    this->ContextViewer->GetSceneRenderer(),
    startPoint, startPoint);
  vtkVgSpaceConversion::DisplayToWorldNormalized(
    this->ContextViewer->GetSceneRenderer(),
    endPoint, endPoint);

  double targetExtents[4] =
    {
    startPoint[0] < endPoint[0] ? startPoint[0] : endPoint[0],
    startPoint[1] < endPoint[1] ? startPoint[1] : endPoint[1],
    startPoint[0] > endPoint[0] ? startPoint[0] : endPoint[0],
    startPoint[1] > endPoint[1] ? startPoint[1] : endPoint[1],
    };

  vtkSmartPointer<vtkPoints> rectangularRegionPoints
    = vtkSmartPointer<vtkPoints>::New();
  rectangularRegionPoints->SetNumberOfPoints(5);
  rectangularRegionPoints->InsertPoint(0, targetExtents[0], targetExtents[1], 1.0);
  rectangularRegionPoints->InsertPoint(1, targetExtents[2], targetExtents[1], 1.0);
  rectangularRegionPoints->InsertPoint(2, targetExtents[2], targetExtents[3], 1.0);
  rectangularRegionPoints->InsertPoint(3, targetExtents[0], targetExtents[3], 1.0);
  rectangularRegionPoints->InsertPoint(4, targetExtents[0], targetExtents[1], 1.0);

  this->ContextTracksSelector->RemoveAllSelectors();
  this->ContextTracksSelector->AddSelector(rectangularRegionPoints);

  vtkVgTrackModel* trackModel = this->TrackNode->GetTrackModel();
  trackModel->InitTrackTraversal();

  QList<vtkVgNodeBase*> nodes;
  while (vtkVgTrack* track = trackModel->GetNextTrack().GetTrack())
    {
    bool passes = this->ContextTracksSelector->EvaluatePath(
                    track->GetPoints(), track->GetPointIds());

    if (passes)
      {
      vtkIdType id = track->GetId();

      // Look up the node corresponding to the picked track.
      vtkVgNodeBase* node = this->findNodeByTrackId(id);

      if (node)
        {
        nodes << node;
        }
      }
    }

  this->selectNodes(nodes);
}

//-----------------------------------------------------------------------------
vtkVgNodeBase* vqCore::findNodeByTrackId(vtkIdType id)
{
  QHash<ResultId, vtkVgVideoNode*>::iterator itr;
  itr = this->QueryResultNodes.find(id);
  if (itr == this->QueryResultNodes.end())
    {
    itr = this->ScoringRequestNodes.find(id);
    if (itr == this->ScoringRequestNodes.end())
      {
      itr = this->GroundTruthNodes.find(id);
      if (itr == this->GroundTruthNodes.end())
        {
        // Should never get here.
        return 0x0;
        }
      }
    }

  vtkVgNodeBase* node = *itr;

  return node;
}

//-----------------------------------------------------------------------------
void vqCore::CacheDatabaseQueryResults(vvQueryResult result)
{
  this->DatabaseQueryResultsCache.append(result);
}

//-----------------------------------------------------------------------------
void vqCore::ExtractTracksAndDescriptorsFromCache()
{
  QList<vvDescriptor> descriptors;
  QList<vvTrack> tracks;

  foreach (vvQueryResult result, this->DatabaseQueryResultsCache)
    {
    for (size_t i = 0; i < result.Descriptors.size(); ++i)
      {
      descriptors.append(result.Descriptors[i]);
      }
    for (size_t i = 0; i < result.Tracks.size(); ++i)
      {
      tracks.append(result.Tracks[i]);
      }
    }

  this->DatabaseQueryResultsCache.clear();

  emit this->formulationComplete(descriptors, tracks);
}

//-----------------------------------------------------------------------------
void vqCore::onLeftClick()
{
  if (this->DrawingContour)
    {
    return;
    }

  int* pointerPosition = this->ContextInteractorStyle->GetEndPosition();

  // Check if a video node has been picked at this position.
  vtkVgNodeBase* node = this->ContextViewer->Pick(pointerPosition[0],
                                                  pointerPosition[1],
                                                  0.0);
  // Check for track pick.
  if (!vtkVgVideoNode::SafeDownCast(node))
    {
    vtkIdType picktype;
    vtkRenderer* ren = this->ContextViewer->GetSceneRenderer();
    vtkVgTrackRepresentation* rep
      = this->TrackNode->GetTrackRepresentation();

    int id = rep->Pick(pointerPosition[0], pointerPosition[1], ren, picktype);
    if (id == -1)
      {
      this->selectNodes(QList<vtkVgNodeBase*>());
      return;
      }

    // Look up the node corresponding to the picked track.
    node = this->findNodeByTrackId(id);
    if (!node)
      {
      qDebug() << "No result for picked track!";
      return;
      }
    }

  if (this->ClickTimer.isActive())
    {
    if (abs(pointerPosition[0] - this->LastClickPosition[0]) < 5 &&
        abs(pointerPosition[1] - this->LastClickPosition[1]) < 5)
      {
      this->activateNode(*node);
      this->ClickTimer.stop();
      return;
      }
    }

  this->ClickTimer.start(400, this);  // double-click time

  this->LastClickPosition[0] = pointerPosition[0];
  this->LastClickPosition[1] = pointerPosition[1];

  QList<vtkVgNodeBase*> nodes;
  nodes << node;
  this->selectNodes(nodes);
}

//-----------------------------------------------------------------------------
void vqCore::addRasterLayer(QUrl uri)
{
  if (uri.scheme().toLower() == "file")
    {
    // Now initialize the terrain source as currently we are using
    // raster to generate a terrain.
    this->TerrainSource = vtkVgTerrainSource::SmartPtr::New();
    this->TerrainSource->SetDataSource(uri.toEncoded().constData());

    // Add the context to the main viewer scene.
    vtkSmartPointer<vtkVgTerrain> terrain
      = this->TerrainSource->CreateTerrain();
    if (!terrain)
      {
      QMessageBox::warning(0, "viqui",
                           "A problem occurred trying to load the layer file.");

      return;
      }

    terrain->SetNodeReferenceFrame(vtkVgNodeBase::ABSOLUTE_REFERENCE);

    this->ContextSceneRoot->AddChild(terrain);

    // \NOTE: Setting the matrix on context video root.
    if (this->ContextVideoRoot)
      {
      this->ContextVideoRoot->SetMatrix(
        this->TerrainSource->GetCoordinateTransformMatrix());
      }

    this->update();

    this->ContextViewer->ResetView();

    // cache the bounds of the terrain (context) for future use (resetting view)
    static_cast<vtkVgNodeBase*>(terrain)->GetBounds(this->TerrainBounds);

    this->resetView();

    // Might (in the future?) want to do updateSources() instead, if have some
    // other data already loaded/rendered by the time we add context data,
    // perhaps other context data?  But for now, would result in another load
    // and render immediately after having just done so for the context.
    this->postRender();
    }
  else
    {
    QMessageBox::critical(0, "viqui",
                          "Non-local context files are not supported.");
    }
}

//-----------------------------------------------------------------------------
void vqCore::updateSources()
{
  if (this->updateTerrainSource())
    {
    this->postRender();
    }
}

//-----------------------------------------------------------------------------
bool vqCore::updateTerrainSource()
{
  if (this->TerrainSource)
    {
    this->TerrainSource->SetImageLevel(-1);

    // Update terrain source.
    this->TerrainSource->SetVisibleScale(this->ViewScaleFactor
      * this->ContextViewer->GetCurrentScale());

    // \TODO: We need to convert these extents to world space.
    // works for now because we work in the coordinates of the terrain
    double viewExtents[4];
    this->ContextViewer->GetCurrentViewExtents(viewExtents);
    int extents[4] =
      {
      static_cast<int>(floor(viewExtents[0])),
      static_cast<int>(ceil(viewExtents[1])),
      static_cast<int>(floor(viewExtents[2])),
      static_cast<int>(ceil(viewExtents[3]))
      };

    this->TerrainSource->SetVisibleExtents(extents);
    this->TerrainSource->Update();

    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void vqCore::setVideoProviders(const QList<QUrl>& providers)
{
  this->VideoArchive.reset(new vgKwaArchive);
  qtUtil::mapBound(providers, this->VideoArchive.data(),
                   &vgKwaArchive::addSource);
}

//-----------------------------------------------------------------------------
void vqCore::setQueryServer(const QUrl& server)
{
  this->QueryServer = server;
}

//-----------------------------------------------------------------------------
bool vqCore::isQuerySessionActive() const
{
  return this->ActiveQueryParser;
}

//-----------------------------------------------------------------------------
void vqCore::addQueryResult(const vvQueryResult& queryResult)
{
  if (this->QueryResults.contains(queryResult.InstanceId))
    {
    // If we have already seen this result, just update its score
    vvQueryResult& existingResult = this->QueryResults[queryResult.InstanceId];
    this->QueryScoreMap.remove(existingResult.Rank,
                               existingResult.InstanceId);
    existingResult.Rank = queryResult.Rank;
    existingResult.RelevancyScore = queryResult.RelevancyScore;
    existingResult.PreferenceScore = queryResult.PreferenceScore;
    this->QueryScoreMap.insert(existingResult.Rank,
                               existingResult.InstanceId);
    return;
    }

  this->QueryScoreMap.insert(queryResult.Rank,
                             queryResult.InstanceId);
  this->QueryResults.insert(queryResult.InstanceId, queryResult);
}

//-----------------------------------------------------------------------------
void vqCore::addGroundTruthResult(const vvQueryResult& queryResult)
{
  this->GroundTruthScoreMap.insert(queryResult.RelevancyScore,
                                   queryResult.InstanceId);
  this->GroundTruthResults.insert(queryResult.InstanceId, queryResult);
}

//-----------------------------------------------------------------------------
void vqCore::displayResults(int count, int first)
{
  this->StatusManager->setStatusText(this->InteractionStatusSource,
                                     "Updating result display...");

  this->resetQueryResults(false, false);

  QMultiMap<ResultRank, ResultId>::const_iterator
  iter = this->QueryScoreMap.begin(),
  end = this->QueryScoreMap.end();

  // Skip to start
  while (first && iter != end)
    {
    ++iter;
    --first;
    }

  count -= this->QueryResultNodes.size();

  // Add all starred results - they will count against the total only if they
  // are in the top count results
  for (QSet<ResultId>::iterator itr = this->StarResults.begin(),
       end = this->StarResults.end();
       itr != end; ++itr)
    {
    if (vtkVgVideoNode* node = this->addResult(*itr, false))
      {
      node->SetIsStarResult(true);
      }
    }

  // Add nodes
  while (count > 0 && iter != end)
    {
    ResultId iid = iter.value();
    if (this->QueryResultNodes.contains(iid))
      {
      if (this->QueryResultNodes[iid]->GetIsStarResult())
        {
        --count; // Starred result, so counts against our total
        }
      }
    else if (this->addResult(iid, true))
      {
      --count;
      }
    ++iter;
    }

  this->updateResultScoreIndicators();

  // (Re)add the scoring requests
  foreach (const auto iid, this->ScoringRequests)
    {
    this->createScoringRequestNode(iid);
    }

  emit this->StatusManager->setStatusText(this->InteractionStatusSource);
  emit this->resultsUpdated();
}

//-----------------------------------------------------------------------------
vtkVgVideoNode* vqCore::addResult(ResultId iid, bool useSpatialFilter)
{
  if (!this->QueryResultNodes.contains(iid) && this->resultPassesFilters(iid))
    {
    vtkVgVideoNode* node = 0;
    if (this->ScoringRequestNodes.contains(iid))
      {
      node = this->ScoringRequestNodes[iid];
      }
    else
      {
      vtkSmartPointer<vtkVgNodeBase> transformNode =
        this->addResultNode(this->QueryResults[iid], node, useSpatialFilter);
      if (transformNode)
        {
        this->ContextVideoRoot->AddChild(transformNode);
        }
      }
    if (node)
      {
      node->SetIsNormalResult(true);
      this->QueryResultNodes.insert(iid, node);
      this->QueryResultNodesDirty = true;
      return node;
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vqCore::displayGroundTruthResults(int count, int first)
{
  this->StatusManager->setStatusText(this->InteractionStatusSource,
                                     "Updating result display...");

  QMultiMap<double, ResultId>::const_iterator
  iter = this->GroundTruthScoreMap.end(),
  end = this->GroundTruthScoreMap.begin();

  // Skip to start
  while (first && iter != end)
    {
    --iter;
    --first;
    }

  count -= this->GroundTruthNodes.size();

  // Add nodes
  while (count > 0 && iter != end)
    {
    ResultId iid = (--iter).value();
    if (!this->GroundTruthNodes.contains(iid))
      {
      vtkVgVideoNode* node = 0;
      vtkSmartPointer<vtkVgNodeBase> transformNode =
        this->addResultNode(this->GroundTruthResults[iid], node, true,
                            true, this->GroundTruthEventType);

      if (transformNode)
        {
        this->ContextVideoRoot->AddChild(transformNode);
        }

      if (node)
        {
        this->GroundTruthNodes.insert(iid, node);
        this->GroundTruthNodesDirty = true;
        --count;
        }
      }
    }

  emit this->StatusManager->setStatusText(this->InteractionStatusSource);
  emit this->resultsUpdated();
}

//-----------------------------------------------------------------------------
bool vqCore::findResult(ResultId iid)
{
  QHash<ResultId, vvQueryResult>::iterator itr = this->QueryResults.find(iid);
  if (itr == this->QueryResults.end())
    {
    return false;
    }

  // Look up in the currently displayed set of nodes
  vtkVgVideoNode* node = this->QueryResultNodes.value(itr.key());
  bool changed = false;

  // Add a new node if the result exists, but is not displayed
  if (!node)
    {
    node = this->addResult(itr.key(), false);
    if (!node)
      {
      return false; // Result does not pass filters?
      }
    changed = true;
    }

  // Update
  if (changed)
    {
    emit this->resultsUpdated();
    this->queryFinished(false); // hack...
    }

  QList<vtkVgNodeBase*> nodes;
  nodes << node;
  this->selectNodes(nodes);

  return true;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgNodeBase>
vqCore::addResultNode(const vvQueryResult& queryResult, vtkVgVideoNode*& node,
                      bool /*useSpatialFilter*/,
                      bool isGroundTruth, int eventTypeFilter)
{
  // \TODO: Here we need to connect to video api and fetch the first frame too.
  // That means that we would need to make a video node at some point with a
  // query data source.

  // Create a new video archive source.
  vtkVgVideoModel0::SmartPtr
  videoModel(vtkVgVideoModel0::SmartPtr::New());
  vtkVgVideoRepresentation0::SmartPtr
  videoRep(vtkVgVideoRepresentation0::SmartPtr::New());
  vqArchiveVideoSource::SmartPtr
  arVideoSource(vqArchiveVideoSource::SmartPtr::New());

  vtkVgVideoNode* videoNode = vtkVgVideoNode::New();

  vtkVgEventModel::SmartPtr
  eventModel(vtkVgEventModel::SmartPtr::New());
  vtkVgEventRegionRepresentation::SmartPtr
  eventRep(vtkVgEventRegionRepresentation::SmartPtr::New());

  videoNode->SetNodeReferenceFrame(vtkVgNodeBase::RELATIVE_REFERENCE);
  std::string missionId = queryResult.MissionId;
  if (missionId == "-undefined-")
    {
    missionId.clear();
    }
  arVideoSource->SetMissionId(missionId.c_str());
  arVideoSource->SetStreamId(queryResult.StreamId.c_str());

  arVideoSource->SetTimeRange(queryResult.StartTime, queryResult.EndTime);
  arVideoSource->SetRequestedPadding(1.0e6 * vqSettings().resultClipPadding());

  if (arVideoSource->AcquireVideoClip(this->VideoArchive.data()) == VTK_OK)
    {
    videoNode->SetHasVideoData(true);
    }

  // For now, we are only using the spatial filtering provided by the back-end
  /*
  if (useSpatialFilter && !this->filterSpatially(arVideoSource,
                                                 queryResult.Descriptors))
    {
    qtDebug(vqdFilterResults)
      << "(spatially) filtered out query result"
      << queryResult.InstanceId;
    node = 0;
    return videoNode;
    }
  */

  // setup the eventModel, merging the all descriptors into a single event
  if (queryResult.Descriptors.size() > 0)
    {
    if (isGroundTruth)
      {
      // ground truth results should have a single event type classifier
      if (queryResult.Descriptors[0].Values.size() != 1)
        {
        std::cout << "Ground truth result has multiple or zero classifiers: "
                  << queryResult.InstanceId << '\n';
        node = 0;
        return videoNode;
        }

      if (queryResult.Descriptors[0].Values[0].size() != 2)
        {
        std::cout << "Ground truth result with unknown classifier structure: "
                  << queryResult.InstanceId << '\n';
        node = 0;
        return videoNode;
        }

      if (queryResult.Descriptors[0].Values[0][1] != 1.0f)
        {
        std::cout << "Warning: Ground truth classifier probability is not "
                  << "1.0: " << queryResult.InstanceId << '\n';
        }

      int at = static_cast<int>(queryResult.Descriptors[0].Values[0][0]);
      videoNode->SetActivityType(at);

      // check against filter
      if (eventTypeFilter != -1 && eventTypeFilter != at)
        {
        node = 0;
        return videoNode;
        }
      }

    vtkSmartPointer<vtkVgEventBase> eventBase =
      vtkSmartPointer<vtkVgEventBase>::New();

    vtkVgTimeStamp startFrame(true);  // initialize to MaxTime
    vtkVgTimeStamp endFrame(false);   // initialize to MinTime

    const double yDim = arVideoSource->GetVideoHeight();
    foreach (vvDescriptor descriptor, queryResult.Descriptors)
      {
      if (descriptor.Region.size() > 0)
        {
        vvDescriptorRegionMap::const_iterator regionIter;
        for (regionIter = descriptor.Region.begin();
             regionIter != descriptor.Region.end(); regionIter++)
          {
          if (regionIter->TimeStamp < startFrame)
            {
            startFrame = regionIter->TimeStamp;
            }
          if (endFrame < regionIter->TimeStamp)
            {
            endFrame = regionIter->TimeStamp;
            }
          double region[8];
          // clockwise from top left
          region[0] = regionIter->ImageRegion.TopLeft.X;
          region[1] = yDim - regionIter->ImageRegion.TopLeft.Y - 1;
          region[2] = regionIter->ImageRegion.BottomRight.X;
          region[3] = region[1];
          region[4] = region[2];
          region[5] = yDim - regionIter->ImageRegion.BottomRight.Y - 1;
          region[6] = region[0];
          region[7] = region[5];

          eventBase->AddRegion(regionIter->TimeStamp, 4, region);
          }
        }
      else
        {
        // Handle regionless events; add a region for the first and last frames
        // (if they are different)
        // TODO: Add a region for every frame between start and end frames or
        //       add region interpolation capability to achieve the same effect
        startFrame.SetTime(static_cast<double>(queryResult.StartTime));
        endFrame.SetTime(static_cast<double>(queryResult.EndTime));

        const double xDim = arVideoSource->GetVideoWidth();
        double region[8];
        // clockwise from top left
        region[0] = 0;
        region[1] = yDim - 1;
        region[2] = xDim - 1;
        region[3] = region[1];
        region[4] = region[2];
        region[5] = 0;
        region[6] = region[0];
        region[7] = region[5];

        eventBase->AddRegion(startFrame, 4, region);
        if (startFrame != endFrame)
          {
          eventBase->AddRegion(endFrame, 4, region);
          }
        }

      // If "MergedDescriptors" descriptor, no need to process any other
      // regions (note that we expect this to have been the first descriptor)
      if (descriptor.ModuleName == "MergedDescriptors")
        {
        break;
        }
      }

    eventBase->SetId(queryResult.InstanceId);
    eventBase->SetStartFrame(startFrame);
    eventBase->SetEndFrame(endFrame);

    eventModel->AddEvent(eventBase);
    }

  videoModel->SetVideoSource(arVideoSource);
  videoModel->SetUseInternalTimeStamp(1);
  videoModel->SetLooping(1);

  videoModel->SetId(queryResult.InstanceId);

  eventRep->SetEventModel(eventModel);
  videoRep->SetVideoModel(videoModel);
  videoModel->SetEventModel(eventModel);
  videoRep->SetEventRepresentation(eventRep);

  // Set the meta data.
  videoNode->SetVideoRepresentation(videoRep);
  videoNode->SetStreamId(queryResult.StreamId.c_str());
  videoNode->SetMissionId(missionId.c_str());
  videoNode->SetInstanceId(queryResult.InstanceId);
  videoNode->SetRank(queryResult.Rank);
  videoNode->SetRelevancyScore(queryResult.RelevancyScore);
  videoNode->SetPreferenceScore(queryResult.PreferenceScore);
  videoNode->SetUserScore(queryResult.UserScore);
  videoNode->SetTimeRange(queryResult.StartTime, queryResult.EndTime);
  videoNode->SetIsStarResult(queryResult.UserData.Flags.testFlag(
                               vvUserData::Starred));
  videoNode->SetNote(queryResult.UserData.Notes.c_str());

  if (!videoNode->GetHasVideoData())
    {
    node = videoNode;
    return 0;
    }

  vtkVgTransformNode::SmartPtr videoTransformNode(
    vtkVgTransformNode::SmartPtr::New());
  videoTransformNode->SetName("VideoTransform");
  videoTransformNode->AddChild(videoNode);

  this->addTrackToContextTrackModel(videoNode);

  node = videoNode;
  return videoTransformNode;
}

//-----------------------------------------------------------------------------
void vqCore::createScoringRequestNode(ResultId iid)
{
  // Create result node, if we don't have one already
  if (!this->ScoringRequestNodes.contains(iid))
    {
    // Create the node
    vtkVgVideoNode* node = 0;
    if (this->QueryResultNodes.contains(iid))
      {
      node = this->QueryResultNodes[iid];
      }
    else
      {
      vtkSmartPointer<vtkVgNodeBase> transformNode =
        this->addResultNode(this->QueryResults[iid], node, false);
      if (transformNode)
        {
        this->ContextVideoRoot->AddChild(transformNode);
        }
      }
    if (node)
      {
      node->SetIsRefinementResult(true);
      this->ScoringRequestNodes.insert(iid, node);
      this->ScoringRequestNodesDirty = true;
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::unregisterAllNodes()
{
  foreach (vtkVgVideoNode* node, this->ScoringRequestNodes)
    {
    // Make sure we don't double-delete a node.
    if (!this->QueryResultNodes.contains(node->GetInstanceId()))
      {
      node->Delete();
      }
    }
  foreach (vtkVgVideoNode* node, this->QueryResultNodes)
    {
    node->Delete();
    }
  foreach (vtkVgVideoNode* node, this->GroundTruthNodes)
    {
    node->Delete();
    }
}

//-----------------------------------------------------------------------------
void  vqCore::layoutResults()
{
  // Make sure that videos are sorted and rank has been updated.
  this->queryResultNodes();
  this->scoringRequestNodes();

  int numberOfChildren = this->ContextVideoRoot->GetNumberOfChildren();

  this->LayoutNodes.clear();
  std::vector<vtkVgNodeBase::SmartPtr>        videoTransformNodes;
  std::vector<vtkVQBlastLayoutNode::SmartPtr> overlappingLayoutNodes;

  // Make sure the tracks are updated *after* video nodes.
  disconnect(this, SIGNAL(playedOrStopped(vtkVgVideoNode&)),
             this, SLOT(updateTrackVisibility()));

  // Here is how the algorithm works...
  // Pick a node, test overlap with all the nodes of other layout nodes.
  // First overlap decide which layout node this node will belong to.
  // If none found then create a new layout node and add this node as child.

  // First we need to gather the layout nodes and video transform nodes attached
  // to the context video root. If we remove any children from context video
  // root the iterator gets invalid.
  for (int i = 0; i < numberOfChildren; ++i)
    {
    vtkVgNodeBase* node = this->ContextVideoRoot->GetChild(i);

    if (!node)
      {
      continue;
      }

    // Do not consider layout nodes.
    if (vtkVQBlastLayoutNode::SmartPtr layoutNode =
          vtkVQBlastLayoutNode::SafeDownCast(node))
      {
      this->LayoutNodes.push_back(layoutNode);
      continue;
      }
    else if (vtkVgTransformNode::SmartPtr transformNode =
               vtkVgTransformNode::SafeDownCast(node))
      {
      videoTransformNodes.push_back(transformNode);
      }
    else if (vtkVgVideoNode::SafeDownCast(node))
      {
      std::cout << "WARNING: A video node should not exist without "
                << "having a transform node as its parent.";
      continue;
      }
    else
      {
      continue;
      }
    }

  const int numberOfVideoTransforms
    = static_cast<int>(videoTransformNodes.size());
  for (int i = 0; i < numberOfVideoTransforms; ++i)
    {
    // For each video transform.
    overlappingLayoutNodes.clear();

    const double* nodeBounds = videoTransformNodes[i]->GetBounds();

    int numberOfLayoutNodes = static_cast<int>(this->LayoutNodes.size());

    for (int j = 0; j < numberOfLayoutNodes; ++j)
      {
      int numberOfLayoutChildren = this->LayoutNodes[j]->GetNumberOfChildren();

      for (int k = 0; k < numberOfLayoutChildren; ++k)
        {
        const double* otherBounds
          = this->LayoutNodes[j]->GetChild(k)->GetBounds();

        // As soon as we find a children of a layout node overlapping with current
        // node we assign the group to this video transform node.
        if (!((nodeBounds[0] > otherBounds[1]) ||
              (nodeBounds[3] < otherBounds[2]) ||
              (nodeBounds[1] < otherBounds[0]) ||
              (nodeBounds[2] > otherBounds[3])))
          {
          // As soon as we hit one of the children of this layout node
          // look for collision with other layout nodes.
          overlappingLayoutNodes.push_back(this->LayoutNodes[j]);
          break;
          }
        }
      }

    if (overlappingLayoutNodes.size() >= 1)
      {
      // If just one, add this video transform to the this one layout node.
      if (overlappingLayoutNodes.size() == 1)
        {
        overlappingLayoutNodes[0]->AddChild(videoTransformNodes[i]);
        }
      else
        {
        // We would have to merge all the overlapping nodes into one.
        // For simplicity we can just collapse all other layout nodes to
        // the first one then add this video transform node to the first one.
        for (size_t n = 1; n < overlappingLayoutNodes.size(); ++n)
          {
          std::vector<vtkVgNodeBase::SmartPtr> transferNodes =
            overlappingLayoutNodes[n]->GetChildren();

          for (size_t m = 0; m < transferNodes.size(); ++m)
            {
            // \NOTE: We should probably remove these nodes as scene nodes from
            // this layout node.
            overlappingLayoutNodes[0]->AddChild(transferNodes[m]);
            }

          // Now remove this layout node from the scene.
          this->ContextVideoRoot->RemoveChild(overlappingLayoutNodes[n]);

          // But now we have to update layoutNodes as well since we took out the other
          // layout nodes.
          this->LayoutNodes.erase(std::remove(this->LayoutNodes.begin(),
            this->LayoutNodes.end(), overlappingLayoutNodes[n]),
            this->LayoutNodes.end());
          }

        overlappingLayoutNodes[0]->AddChild(videoTransformNodes[i]);
        }


      int currentNumberOfLayoutNodes
        = static_cast<int>(this->LayoutNodes.size());
      for (int ii = 0; ii < currentNumberOfLayoutNodes; ++ii)
        {
        this->LayoutNodes[ii]->AddSceneNode(videoTransformNodes[i]);

        // TODO: Get rid of this hack once the layout node manages layout updates
        // updates automatically.
        this->LayoutNodes[ii]->SetLayoutMode(
          this->LayoutNodes[ii]->GetLayoutMode());
        }

      }
    else
      {
      vtkVQBlastLayoutNode::SmartPtr newLayoutNode =
        vtkVQBlastLayoutNode::SmartPtr::New();
      newLayoutNode->AddChild(videoTransformNodes[i]);

      connect(this, SIGNAL(playedOrStopped(vtkVgVideoNode&)),
              newLayoutNode.GetPointer(),
              SLOT(OnPlayStop(vtkVgVideoNode&)));

      connect(this, SIGNAL(NextInStack(vtkVgNodeBase&)),
              newLayoutNode.GetPointer(),
              SLOT(OnNextInStack(vtkVgNodeBase&)));
      connect(newLayoutNode.GetPointer(),
              SIGNAL(VisibilityChanged(vtkVgNodeBase&)),
              this, SIGNAL(FocusChanged(vtkVgNodeBase&)));

      connect(this, SIGNAL(updatedResultScoreIndicators()),
              newLayoutNode.GetPointer(),
              SLOT(BuildStackRepresentationBar()));

      vtkConnect(newLayoutNode->GetStackSelectionWidget(),
                 vtkCommand::WidgetActivateEvent,
                 this, SLOT(onSelectInStackLayoutWidget(vtkObject*)));

      // set actor and colormap for StackSelectionWidget
      newLayoutNode->GetStackSelectionWidget()->SetInteractor(
        this->ContextViewer->GetRenderWindowInteractor());
      vtkVgSelectionListRepresentation::SafeDownCast(
        newLayoutNode->GetStackSelectionWidget()->
          GetSelectionRepresentation())->SetLookupTable(this->getLookupTable());

      for (int j = 0; j < numberOfLayoutNodes; ++j)
        {
        int numberOfLayoutChildren
          = this->LayoutNodes[j]->GetNumberOfChildren();

        for (int k = 0; k < numberOfLayoutChildren; ++k)
          {
          newLayoutNode->AddSceneNode(this->LayoutNodes[j]->GetChild(k));
          }

        // Also add this video transform to all other.
        this->LayoutNodes[j]->AddSceneNode(videoTransformNodes[i]);
        }

      this->LayoutNodes.push_back(newLayoutNode);
      this->ContextVideoRoot->AddChild(newLayoutNode);
      }

    if (!videoTransformNodes[i]->GetParent())
      {
      std::cerr << "WARNING: Every video transform should belong to a"
        << " layout node. Application may not work correctly. " << std::endl;
      }
    } // for (int i=0; i <numberOfVideoTransforms; ++i)

  // Update track visibility for nodes that have been hidden or shown
  // due to the playing or stopping of a video.
  connect(this, SIGNAL(playedOrStopped(vtkVgVideoNode&)),
          this, SLOT(updateTrackVisibility()));
}

//-----------------------------------------------------------------------------
void vqCore::updateScene()
{
  // Check for overlapping nodes and move nodes around if necessary.
  this->update();
  this->layoutResults();
  this->update();

  static bool first = true;
  if (first)
    {
    first = false;
    this->resetView();
    }
  this->updateSources();

  // color outline
  this->updateResultScoreIndicators();

  this->updateLOD(true);

  // Update visibility.
  this->setShowVideoClipsOnContext(this->ShowVideoClipsOnContext);
  this->setShowVideoOutlinesOnContext(this->ShowVideoOutlinesOnContext);
}

//-----------------------------------------------------------------------------
void vqCore::queryFinished(bool requestScoring)
{
  if (this->ProcessingDatabaseQuery)
    {
    this->ProcessingDatabaseQuery = false;
    this->ExtractTracksAndDescriptorsFromCache();
    return;
    }

  // Note:  This is temporary code to facilitate doing Refine in presence of an
  // IQR model in the submitted query, without teasing the user with the
  // initial result set; see commit for necessary code to remove when the
  // the system supports IQR modified results on initial query
  if (this->SkipInitialDisplayCounter > 0)
    {
    this->SkipInitialDisplayCounter--;
    if (this->SkipInitialDisplayCounter == 1)
      {
      this->requestRefinement(vqSettings().iqrRefinementSetSize());
      }
    else if (this->SkipInitialDisplayCounter == 0)
      {
      emit initialRefinementRequested();
      }
    this->StatusManager->setStatusText(this->QuerySessionStatusSource,
                                       "Refining query based on IQR Model...");
    return;
    }

  // Apparently we are waiting longer than expected hence show busy wait
  vqScopedOverrideCursor busy;

  if (!this->ReceivingGroundTruthResults)
    {
    this->displayResults(vqSettings().resultPageCount());
    }

  if (!this->GroundTruthResults.empty())
    {
    this->displayGroundTruthResults(vqSettings().resultPageCount());
    }

  // If this is a result set (not a score-request set), transfer the status
  // ownership. This will allow the session to display other, transient messages
  // (e.g. IQR related), while we will preserve the most recent completion
  // message.
  if (!requestScoring)
    {
    this->transferQueryStatus();
    }

  this->updateScene();

  if (!requestScoring)
    {
    emit this->queryResultsAvailabilityChanged(true);
    }

  if (this->QueryResults.isEmpty())
    {
    QMessageBox::information(0, "Sorry",
                             "No results were found for your query");
    }

  emit this->resultSetComplete(!!this->ActiveQueryParser, requestScoring);
}

//-----------------------------------------------------------------------------
void vqCore::updateResultScoreIndicators()
{
  this->recomputeNodeColors();
  this->updateTrackColors();

  this->update();

  emit this->updatedResultScoreIndicators();
}

//-----------------------------------------------------------------------------
void vqCore::setUserScore(ResultId iid, int score)
{
  if (this->QueryResults.contains(iid))
    {
    vvIqr::Classification c = vvIqr::UnclassifiedExample;
    if (score == vvIqr::PositiveExample)
      c = vvIqr::PositiveExample;
    if (score == vvIqr::NegativeExample)
      c = vvIqr::NegativeExample;
    this->QueryResults[iid].UserScore = c;
    }
}

//-----------------------------------------------------------------------------
void vqCore::setResultNote(ResultId iid, const QString& note)
{
  if (this->QueryResults.contains(iid))
    {
    this->QueryResults[iid].UserData.Notes = stdString(note);
    }
}

//-----------------------------------------------------------------------------
void vqCore::setResultStarred(ResultId iid, bool starred)
{
  if (this->QueryResults.contains(iid))
    {
    if (starred)
      {
      this->QueryResults[iid].UserData.Flags |= vvUserData::Starred;
      }
    else
      {
      this->QueryResults[iid].UserData.Flags &= ~vvUserData::Starred;
      }
    }
}

//-----------------------------------------------------------------------------
vvQueryResult* vqCore::getResult(ResultId iid)
{
  QHash<ResultId, vvQueryResult>::iterator itr = this->QueryResults.find(iid);
  if (itr != this->QueryResults.end())
    {
    return &itr.value();
    }

  // result not found
  return 0;
}

//-----------------------------------------------------------------------------
vvQueryResult* vqCore::getGroundTruthResult(ResultId iid)
{
  QHash<ResultId, vvQueryResult>::iterator itr =
    this->GroundTruthResults.find(iid);

  if (itr != this->QueryResults.end())
    {
    return &itr.value();
    }

  // result not found
  return 0;
}

//-----------------------------------------------------------------------------
vtkVgVideoNode* vqCore::getResultNode(ResultId iid)
{
  if (vtkVgVideoNode* node = this->QueryResultNodes.value(iid))
    {
    return node;
    }
  // return null if there is no node for the id
  return this->ScoringRequestNodes.value(iid);
}


//-----------------------------------------------------------------------------
void vqCore::selectResult(ResultId iid)
{
  if (vtkVgVideoNode* node = this->getResultNode(iid))
    {
    QList<vtkVgNodeBase*> list;
    list << node;
    this->selectNodes(list);
    }
}


//-----------------------------------------------------------------------------
void vqCore::activateResult(ResultId iid)
{
  if (vtkVgVideoNode* node = this->getResultNode(iid))
    {
    this->activateNode(*node);
    }
}


//-----------------------------------------------------------------------------
void vqCore::updateLayoutStacks()
{
  std::vector<vtkVQBlastLayoutNode::SmartPtr>::const_iterator layoutIter;
  for (layoutIter = this->LayoutNodes.begin();
       layoutIter != this->LayoutNodes.end(); layoutIter++)
    {
    layoutIter->GetPointer()->UpdateStackLayout();
    }

  this->update();
}

//-----------------------------------------------------------------------------
void vqCore::updateLayoutStackBarLocations()
{
  std::vector<vtkVQBlastLayoutNode::SmartPtr>::const_iterator layoutIter;
  for (layoutIter = this->LayoutNodes.begin();
       layoutIter != this->LayoutNodes.end(); layoutIter++)
    {
    layoutIter->GetPointer()->UpdateStackRepresentationBarLocation();
    }
}

//-----------------------------------------------------------------------------
void vqCore::updateStackLayoutForVideoPlaying(vtkVgNodeBase& videoNode)
{
  if (videoNode.GetParent())
    {
    vtkVQBlastLayoutNode* blastLayoutNode =
      vtkVQBlastLayoutNode::SafeDownCast(videoNode.GetParent()->GetParent());
    if (blastLayoutNode->GetLayoutMode() == vtkVQBlastLayoutNode::STACK)
      {
      blastLayoutNode->GetStackSelectionWidget()->EnabledOff();
      }
    }
}


//-----------------------------------------------------------------------------
void vqCore::videoStopped(vtkVgNodeBase& videoNode)
{
  // is this a node in a layout AND, is the layout in stack mode;
  // if no, do nothing
  if (videoNode.GetParent())
    {
    vtkVQBlastLayoutNode* blastLayoutNode =
      vtkVQBlastLayoutNode::SafeDownCast(videoNode.GetParent()->GetParent());
    if (blastLayoutNode &&
        blastLayoutNode->GetLayoutMode() == vtkVQBlastLayoutNode::STACK)
      {
      this->CachedLayoutNode = blastLayoutNode;
      connect(this, SIGNAL(forcedRender()),
              this, SLOT(updateStackLayoutForVideoStopped()));
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::updateStackLayoutForVideoStopped()
{
  // should just have to renable the bar, as we're returning to the
  // camera we were at when we decided to "play"
  this->CachedLayoutNode->UpdateStackRepresentationBarLocation();
  this->CachedLayoutNode->GetStackSelectionWidget()->EnabledOn();

  // disconnet so we don't repeat this until needed again
  disconnect(this, SIGNAL(forcedRender()),
             this, SLOT(updateStackLayoutForVideoStopped()));
}

//-----------------------------------------------------------------------------
void vqCore::onSelectInStackLayoutWidget(vtkObject* caller)
{
  // find the layout node that with widget == caller (ugly, but will do for now)
  std::vector<vtkVQBlastLayoutNode::SmartPtr>::const_iterator layoutIter;
  for (layoutIter = this->LayoutNodes.begin();
       layoutIter != this->LayoutNodes.end(); layoutIter++)
    {
    if (layoutIter->GetPointer()->GetStackSelectionWidget() == caller)
      {
      layoutIter->GetPointer()->MoveToSelectedStackItem();
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::onRightButtonPress()
{
  // record the right click position for future reference
  this->ContextInteractorStyle->GetInteractor()
  ->GetEventPosition(this->LastRightClickPosition);
}

//-----------------------------------------------------------------------------
void vqCore::onRightButtonRelease()
{
  if (this->DrawingContour)
    {
    this->completeRegion();
    return;
    }

  this->onContextMenuEvent();
}

//-----------------------------------------------------------------------------
void vqCore::onMouseWheelForward()
{
  int pointerPosition[2];
  this->ContextInteractorStyle->GetInteractor()->GetEventPosition(pointerPosition);

  vtkVgNodeBase* node = this->ContextViewer->Pick(pointerPosition[0],
                                                  pointerPosition[1], 0.0);
  bool eventHandled = false;
  if (node)
    {
    emit this->scrolledForward(*node, &eventHandled);
    }

  // allow the interactor style to see the event if no one intercepted it
  if (!eventHandled)
    {
    this->ContextInteractorStyle->OnMouseWheelForward();
    }
}

//-----------------------------------------------------------------------------
void vqCore::onMouseWheelBackward()
{
  int pointerPosition[2];
  this->ContextInteractorStyle->GetInteractor()->GetEventPosition(pointerPosition);

  vtkVgNodeBase* node = this->ContextViewer->Pick(pointerPosition[0],
                                                  pointerPosition[1], 0.0);
  bool eventHandled = false;
  if (node)
    {
    emit this->scrolledBackward(*node, &eventHandled);
    }

  // allow the interactor style to see the event if no one intercepted it
  if (!eventHandled)
    {
    this->ContextInteractorStyle->OnMouseWheelBackward();
    }
}

//-----------------------------------------------------------------------------
void vqCore::onPKeyPress()
{
  int pointerPosition[2];
  this->ContextInteractorStyle->GetInteractor()->GetEventPosition(pointerPosition);

  // Check if a node has been picked at this position.
  vtkVgNodeBase* node = this->ContextViewer->Pick(pointerPosition[0],
                                                  pointerPosition[1], 0.0);

  if (node)
    {
    emit this->NextInStack(*node);
    }
}

//-----------------------------------------------------------------------------
void vqCore::onContextMenuEvent()
{
  int pos[2];
  this->ContextInteractorStyle->GetInteractor()->GetEventPosition(pos);

  // don't show a context menu at the end of a drag
  if (abs(pos[0] - this->LastRightClickPosition[0]) > 5 ||
      abs(pos[1] - this->LastRightClickPosition[1]) > 5)
    {
    return;
    }

  vtkVgNodeBase* node = this->ContextViewer->Pick(pos[0], pos[1], 0.0);
  if (!node)
    {
    return;
    }

  // Register a menu now
  QMenu menu(this->RenderWidget);
  QAction* picked = 0;
  const int height = this->ContextInteractorStyle->GetInteractor()->GetSize()[1];

  // Get screen mouse position (adjusted for zero-based value)
  const QPoint menuLocation =
    this->RenderWidget->mapToGlobal(QPoint(pos[0], height - pos[1] - 1));

  QAction* copyLocation = menu.addAction("&Copy Location");

  // search for a parent layout node
  vtkVgNodeBase* origNode = node;
  for (; node != 0; node = node->GetParent())
    {
    if (vtkVQBlastLayoutNode* blastNode = vtkVQBlastLayoutNode::SafeDownCast(node))
      {
      // don't attempt to change layouts on a 'zoomed' node
      if (blastNode->GetNodeFocused())
        {
        return;
        }

      int layoutMode = blastNode->GetLayoutMode();

      menu.addSeparator();
      QAction* zSortLayout = menu.addAction("Z Sort");
      zSortLayout->setCheckable(true);
      zSortLayout->setChecked(layoutMode == vtkVQBlastLayoutNode::Z_SORT);

      QAction* stackLayout = menu.addAction("Stack");
      stackLayout->setCheckable(true);
      stackLayout->setChecked(layoutMode == vtkVQBlastLayoutNode::STACK);

      QAction* blastLayout = menu.addAction("Blast");
      blastLayout->setCheckable(true);
      blastLayout->setChecked(layoutMode == vtkVQBlastLayoutNode::BLAST);

      // go back to Qt widget coords
      pos[1] = this->ContextInteractorStyle->GetInteractor()->GetSize()[1] -
               pos[1] - 1;

      // show the menu
      picked = menu.exec(menuLocation);

      int newLayoutMode;
      if (picked == zSortLayout)
        {
        newLayoutMode = vtkVQBlastLayoutNode::Z_SORT;
        }
      else if (picked == stackLayout)
        {
        newLayoutMode = vtkVQBlastLayoutNode::STACK;
        }
      else if (picked == blastLayout)
        {
        newLayoutMode = vtkVQBlastLayoutNode::BLAST;
        }
      else
        {
        // nothing picked
        return;
        }

      if (vtkVgGroupNode::SmartPtr parent = origNode->GetParent())
        {
        if (vtkVQBlastLayoutNode::SmartPtr layoutNode = vtkVQBlastLayoutNode::SafeDownCast(
                                                          parent->GetParent()))
          {
          layoutNode->SetLayoutMode(newLayoutMode);
          this->update();
          }
        }
      return;
      }
    }

  // show the menu
  picked = menu.exec(menuLocation);
  if (picked == copyLocation)
    {
    vgGeocodedCoordinate coord;
    coord.GCS = 4326;
    this->transformDisplayToLatLon(pos[0], pos[1],
                                   coord.Latitude, coord.Longitude);

    QString coordString = vgGeodesy::coordString(coord);
    QApplication::clipboard()->setText(
      coordString.left(coordString.indexOf('(') - 1));
    return;
    }
}

//-----------------------------------------------------------------------------
void vqCore::updateLOD(bool override)
{
  this->lodToggleVisibility(0.20, 1, true, false, override);
  this->lodToggleVisibility(0.50, 0, false, true, override);
}

//-----------------------------------------------------------------------------
void vqCore::activateNode(vtkVgNodeBase& node)
{
  emit this->activated(node);
}

//-----------------------------------------------------------------------------
void vqCore::selectNodes(QList<vtkVgNodeBase*> nodes)
{
  // Restore normal colors.
  this->updateTrackColors();

  // Set a different color for selected tracks.
  vtkVgTrackModel* trackModel = this->TrackNode->GetTrackModel();
  foreach (vtkVgNodeBase* node, nodes)
    {
    if (vtkVgVideoNode* vnode = vtkVgVideoNode::SafeDownCast(node))
      {
      trackModel->SetTrackColor(vnode->GetInstanceId(), SelectedTrackColor);
      }
    }
  trackModel->Modified();

  emit this->selected(nodes);

  this->update();
}

//-----------------------------------------------------------------------------
void vqCore::setColorMappingMode(vqCore::NodeColorMode mode)
{
  if (this->NodeColorMappingMode != mode)
    {
    this->NodeColorMappingMode = mode;
    emit this->coloringChanged();
    }
}

//-----------------------------------------------------------------------------
void vqCore::setColorShadingMode(qtGradient::InterpolationMode mode)
{
  if (this->NodeColorShadingMode != mode)
    {
    this->NodeColorShadingMode = mode;
    this->reloadColorGradient();
    }
}

//-----------------------------------------------------------------------------
void vqCore::setLutValue(int index, const QColor& color)
{
  double c[] = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
  this->ItemColorLUT->SetTableValue(index, c);
}

//-----------------------------------------------------------------------------
void vqCore::reloadColorGradient()
{
  qtGradient gradient = vqSettings().scoreGradient().gradient();
  gradient.setInterpolationMode(this->NodeColorShadingMode);

  // Build first half of LUT from gradient
  this->ItemColorLUT->SetNumberOfTableValues(200);
  this->ItemColorLUT->SetRange(0.0, 2.0);

  QList<QColor> colors = gradient.render(100);
  for (int i = 0; i < 125; ++i)
    this->setLutValue(i, colors[(i < 100 ? i : 99)]);

  // Fill remainder of table with score-request and ground-truth colors
  // Score-requests blue, ground truth gray (translucent in timelime)
  // \TODO get from user preferences
  const QColor gtColor = QColor::fromRgbF(0.9, 0.9, 0.9, 0.75);
  const QColor srColor = Qt::blue;
  for (int i = 125; i < 200; ++i)
    this->setLutValue(i, (i < 175 ? gtColor : srColor));

  emit this->coloringChanged();
}

//-----------------------------------------------------------------------------
void vqCore::setViewToExtents(double extents[4])
{
  // Sources and LOD will be updated via vtkVQInteractorCallback
  this->ContextInteractorStyle->ZoomToExtents(
    this->ContextViewer->GetSceneRenderer(), extents);
}

//-----------------------------------------------------------------------------
void vqCore::updateNodeColor(vtkVgVideoNode* node)
{
  double color[3];
  this->ItemColorLUT->GetColor(node->GetColorScalar(), color);

  vtkVgVideoRepresentation0* videoRep =
    vtkVgVideoRepresentation0::SafeDownCast(node->GetVideoRepresentation());

  videoRep->SetOutlineColor(color);
  videoRep->SetOutlineStippled(node->GetIsRefinementResult());
}

//-----------------------------------------------------------------------------
void vqCore::colorNodesByRank()
{
  QList<vtkVgVideoNode*> queryResultNodes = this->queryResultNodes();

  // calculate size of visible result set
  int totalVisible = 0;
  int numResultNodes = queryResultNodes.size();
  for (int i = 0; i < numResultNodes; ++i)
    {
    if (queryResultNodes[i]->GetVisibleNodeMask() ==
        vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      ++totalVisible;
      }
    }
  // second pass to compute rank-based node colors
  for (int i = 0, count = 0; i < numResultNodes; ++i)
    {
    vtkVgVideoNode* node = queryResultNodes[i];
    if (node->GetVisibleNodeMask() == vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      double x = static_cast<double>(count++) / totalVisible;
      node->SetColorScalar(x);
      this->updateNodeColor(node);
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::colorNodesByRelativeScore()
{
  QList<vtkVgVideoNode*> queryResultNodes = this->queryResultNodes();

  double minScore = -1;
  double maxScore = -1;

  int numResultNodes = queryResultNodes.size();

  if (numResultNodes != 0)
    {
    minScore = maxScore = queryResultNodes.front()->GetRelevancyScore();
    }

  // calculate the range of scores in the visible result set
  for (int i = 0; i < numResultNodes; ++i)
    {
    vtkVgVideoNode* node = queryResultNodes[i];
    if (node->GetVisibleNodeMask() == vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      minScore = std::min(minScore, node->GetRelevancyScore());
      maxScore = std::max(maxScore, node->GetRelevancyScore());
      }
    }

  // compute score-based relevancy and update node colors
  for (int i = 0; i < numResultNodes; ++i)
    {
    vtkVgVideoNode* node = queryResultNodes[i];
    if (node->GetVisibleNodeMask() == vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      double x = maxScore == minScore
                 ? 1.0
                 : (node->GetRelevancyScore() - minScore) / (maxScore - minScore);

      node->SetColorScalar(1.0 - x);
      this->updateNodeColor(node);
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::colorNodesByAbsoluteScore()
{
  QList<vtkVgVideoNode*> queryResultNodes = this->queryResultNodes();

  int numResultNodes = queryResultNodes.size();

  for (int i = 0; i < numResultNodes; ++i)
    {
    vtkVgVideoNode* node = queryResultNodes[i];
    if (node->GetVisibleNodeMask() == vtkVgNodeBase::VISIBLE_NODE_MASK)
      {
      node->SetColorScalar(1 - node->GetRelevancyScore());
      this->updateNodeColor(node);
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::recomputeNodeColors()
{
  // ground truth results are all the same color
  QList<vtkVgVideoNode*> groundTruthNodes = this->groundTruthNodes();
  for (int i = 0, end = groundTruthNodes.size(); i < end; ++i)
    {
    vtkVgVideoNode* node = groundTruthNodes[i];
    node->SetColorScalar(1.5);
    this->updateNodeColor(node);
    }

  if (this->ReceivingGroundTruthResults)
    {
    return;
    }

  QList<vtkVgVideoNode*> scoringResultNodes = this->scoringRequestNodes();

  switch (this->NodeColorMappingMode)
    {
    case ColorByAbsoluteScore:
      this->colorNodesByAbsoluteScore();
      break;
    case ColorByRelativeScore:
      this->colorNodesByRelativeScore();
      break;
    default:
      this->colorNodesByRank();
      break;
    }

  // feedback results are given the special color scalar value
  int numRefinementResults = scoringResultNodes.size();
  for (int i = 0; i < numRefinementResults; ++i)
    {
    vtkVgVideoNode* node = scoringResultNodes[i];

    // only override color if not part of normal results
    if (!node->GetIsNormalResult())
      {
      node->SetColorScalar(2.0);
      this->updateNodeColor(node);
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::resetView()
{
  double bounds[6];
  this->ContextViewer->GetSceneRenderer()->ComputeVisiblePropBounds(bounds);

  // need to include full terrain, as we may be zoomed in to see only a portion
  if (vtkMath::AreBoundsInitialized(this->TerrainBounds))
    {
    if (this->TerrainBounds[0] < bounds[0])
      {
      bounds[0] = this->TerrainBounds[0];
      }
    if (this->TerrainBounds[1] > bounds[1])
      {
      bounds[1] = this->TerrainBounds[1];
      }
    if (this->TerrainBounds[2] < bounds[2])
      {
      bounds[2] = this->TerrainBounds[2];
      }
    if (this->TerrainBounds[3] > bounds[3])
      {
      bounds[3] = this->TerrainBounds[3];
      }
    }

  this->setViewToExtents(bounds);
}

//-----------------------------------------------------------------------------
void vqCore::resetViewToQueryResults()
{
  if (!this->ContextVideoRoot)
    {
    return;
    }
  double* bounds = this->ContextVideoRoot->GetBounds();
  if (!vtkMath::AreBoundsInitialized(bounds))
    {
    return;
    }
  this->setViewToExtents(bounds);
}

//-----------------------------------------------------------------------------
void vqCore::drawRegion()
{
  this->DrawingContour = true;

  this->StatusManager->setStatusText(this->InteractionStatusSource,
                                     "Drawing region (right-click when done)");

  delete this->Contour;
  this->Contour = new vqContour(this->ContextViewer->GetRenderWindowInteractor());
  this->Contour->Initialize();
  this->Contour->Begin();
}

//-----------------------------------------------------------------------------
void vqCore::setRegion(vgGeocodedPoly region)
{
  delete this->Contour;
  if (region.Coordinate.size())
    this->Contour = this->createStaticContour(region, false);
  else
    this->Contour = 0;
}

//-----------------------------------------------------------------------------
void vqCore::completeRegion()
{
  this->StatusManager->setStatusText(this->InteractionStatusSource);
  this->DrawingContour = false;
  this->Contour->End();

  vtkSmartPointer<vtkMatrix4x4> invertMat = vtkSmartPointer<vtkMatrix4x4>::New();
  invertMat->DeepCopy(this->TerrainSource->GetCoordinateTransformMatrix());
  invertMat->Invert();

  // Transform the points to lat-lon
  vtkPoints* points = this->Contour->GetPolyData()->GetPoints();
  vgGeocodedPoly region;
  region.GCS = 4269;
  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
    double point[4];
    points->GetPoint(i, point);
    vgGeoRawCoordinate gcp;
    vtkVgApplyHomography(point, invertMat, gcp.Longitude, gcp.Latitude);
    region.Coordinate.push_back(gcp);
    }

  emit this->regionComplete(region);
}

//-----------------------------------------------------------------------------
void vqCore::forceRender()
{
  this->RenderPending = false;
  this->ContextViewer->ForceRender(false);
  emit this->forcedRender();
}

//-----------------------------------------------------------------------------
void vqCore::renderLoopOn()
{
  if (!this->ContinuousRender)
    {
    this->UpdateTimer->start(this->UpdateIntervalMSec);
    this->ContinuousRender = true;
    }
}

//-----------------------------------------------------------------------------
void vqCore::renderLoopOff()
{
  if (this->ContinuousRender)
    {
    this->UpdateTimer->stop();
    this->ContinuousRender = false;
    }
}

//-----------------------------------------------------------------------------
void vqCore::selectItemsOnContext()
{
  vtkVgInteractorStyleRubberBand2D* style =
    vtkVgInteractorStyleRubberBand2D::SafeDownCast(
      this->RenderWidget->GetInteractor()->GetInteractorStyle());

  int* startPosition = style->GetStartPosition();
  int* endPosition =  style->GetEndPosition();

  if (this->ShowVideoClipsOnContext)
    {
    this->selectClipsOnContext(startPosition, endPosition);
    }
  else if (this->ShowTracksOnContext)
    {
    this->selectTracksOnContext(startPosition, endPosition);
    }
  else
    {
    // Do nothing.
    }
}

//-----------------------------------------------------------------------------
void vqCore::updateIqrModel()
{
  // request updated IQR model before saving
  // \TODO keep track of any refinement; if none, than no need to get update
  if (this->ActiveQueryParser) // might this be 0?
    {
    this->ActiveQueryParser->updateIqrModel(this->SavedQueryPlan);
    }
}

//-----------------------------------------------------------------------------
void vqCore::saveQueryPlan()
{
  if (!this->SavedQueryPlan.isValid())
    {
    QMessageBox::warning(0, "Viqui", "Unable to save query;"
                         " query is unset or invalid");
    return;
    }

  this->updateIqrModel();

  vqQueryDialog::saveQuery(this->SavedQueryPlan);
}

//-----------------------------------------------------------------------------
void vqCore::saveResults()
{
  if (this->QueryResults.isEmpty())
    {
    QMessageBox::warning(0, "Viqui", "Unable to save results;"
                         " result set is empty");
    return;
    }

  vqExporter* exporter = vqExporterFactory::createNativeFileExporter();
  if (exporter)
    {
    exporter->exportResults(this->QueryResults.values());
    delete exporter;
    }
}

//-----------------------------------------------------------------------------
void vqCore::exportResults(QList<vtkVgVideoNode*> results, QString exporterId)
{
  QList<vvQueryResult> exportResults;
  for (int i = 0; i < results.count(); ++i)
    {
    vtkIdType sid = results[i]->GetInstanceId();
    if (this->QueryResults.contains(sid))
      {
      exportResults.append(this->QueryResults[sid]);
      }
    }

  if (exportResults.isEmpty())
    {
    QMessageBox::warning(0, "Viqui", "Unable to export;"
                         " selected result set is empty");
    }
  else
    {
    vqExporter* exporter = vqExporterFactory::createExporter(exporterId);
    if (exporter)
      {
      exporter->exportResults(exportResults);
      delete exporter;
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::openExternal(QUrl clipUri, QString streamId, double time)
{
  // Get external player application
  QSettings settings;
  settings.beginGroup("ExternalPlayer");
  const QString externalPlayer =
    settings.value("Program", "vsPlay").toString();
  if (externalPlayer.isEmpty())
    {
    QMessageBox::information(qApp->activeWindow(), "Sorry",
                             "External player has not been configured.");
    return;
    }

  // Get options for running external player
  QStringList args = settings.value("Args").toStringList();
  const QString extractClassifiers =
    settings.value("ExtractClassifiers", "no").toString();
  const bool stripResults = settings.value("StripResults", true).toBool();

  // Get query server URI (for building database source)
  QUrl dataUri = vqSettings().queryServerUri();
  dataUri.addQueryItem("Stream", streamId);
  dataUri.addQueryItem("ExtractClassifiers", extractClassifiers);

  // Generate argument list
  args << "--video-file" << clipUri.toLocalFile()
       << "--query-database" << dataUri.toString()
       << "--seek" << QString::number(time, 'f', 3);

  // Extract results with same stream ID
  QList<vvQueryResult> results;
  double visibleThreshold = 1.0;
  const std::string stdStreamId = stdString(streamId);
  foreach (const vvQueryResult& result, this->QueryResults)
    {
    if (result.StreamId == stdStreamId)
      {
      results.append(stripResults ? stripResult(result) : result);
      if (this->QueryResultNodes.contains(result.InstanceId) &&
          !this->QueryResultNodes[result.InstanceId]->GetIsStarResult())
        {
        // Try to determine threshold so that external player will only display
        // results that are in our top <page size> initially
        visibleThreshold = qMin(visibleThreshold, result.RelevancyScore);
        }
      }
    }

  if (!results.isEmpty())
    {
    // Create temporary file for results
    const QString tmpDir =
      settings.value("TmpDir", QDir::tempPath()).toString();
    QTemporaryFile resultFile;
    resultFile.setFileTemplate(tmpDir + "/results_XXXXXX.vsr");
    resultFile.setAutoRemove(false);
    if (resultFile.open())
      {
      // \TODO event set information should be configurable
      vvEventSetInfo info;
      info.Name = "Query Results";
      info.PenColor = Qt::green;
      info.BackgroundColor = Qt::darkGreen;
      info.ForegroundColor = Qt::white;
      info.DisplayThreshold = visibleThreshold;

      // Write results to temporary file
      vvKstWriter writer(resultFile);
      writer << vvHeader::EventSetInfo << info
             << vvHeader::QueryResults << results;

      // Add to argument list
      args << "--descriptors-file" << resultFile.fileName();
      }
    }

  // Start external player
  QProcess process;
  if (!process.startDetached(externalPlayer, args))
    {
    // Something went wrong
    QMessageBox::warning(qApp->activeWindow(), "Sorry",
                         "Failed to start external player.");
    }
}

//-----------------------------------------------------------------------------
static bool eventEarlierThan(vtkVgVideoNode* a, vtkVgVideoNode* b)
{
  return a->GetTimeRange()[0] < b->GetTimeRange()[0];
}

//-----------------------------------------------------------------------------
void vqCore::generateReport(QString path, bool generateVideo)
{
  QFile file(path + '/' + "report.xml");
  if (!file.open(QIODevice::WriteOnly |
                 QIODevice::Text |
                 QIODevice::Truncate))
    {
    qDebug() << file.errorString();
    QMessageBox::warning(0, QString(), "Unable to write report file.");
    return;
    }

  vvReportWriter writer(file);

  vtkSmartPointer<vqArchiveVideoSource> video =
    vtkSmartPointer<vqArchiveVideoSource>::New();

  vtkSmartPointer<vtkVgTerrainSource> terrainSrc;
  vtkSmartPointer<vtkVgTerrain> terrain;

  // Terrain source doesn't support deep copy at the moment
  if (this->TerrainSource && this->TerrainSource->GetDataSource())
    {
    terrainSrc = vtkSmartPointer<vtkVgTerrainSource>::New();
    terrainSrc->SetDataSource(this->TerrainSource->GetDataSource());
    terrain = terrainSrc->CreateTerrain();
    writer.setContext(terrain, terrainSrc,
                      terrainSrc->GetCoordinateTransformMatrix());
    }

  // Sort by start time
  QList<vtkVgVideoNode*> nodes = this->QueryResultNodes.values();
  qSort(nodes.begin(), nodes.end(), eventEarlierThan);

  // Try to estimate the amount of work that we'll be doing
  int total = 0;
  foreach (vtkVgVideoNode* node, nodes)
    {
    if (node->GetIsStarResult())
      {
      vtkVgVideoModel0* vm = node->GetVideoRepresentation()->GetVideoModel();
      vtkVgEventModel* em = vm->GetEventModel();

      if (vtkVgEvent* event = em->GetEvent(node->GetInstanceId()))
        {
        if (!generateVideo)
          {
          ++total;
          }
        else if (event->GetStartFrame().HasFrameNumber() &&
                 event->GetEndFrame().HasFrameNumber())
          {
          total += event->GetEndFrame().GetFrameNumber() -
                   event->GetStartFrame().GetFrameNumber() + 1;
          }
        else
          {
          total += 100;
          }
        }
      }
    }

  QProgressDialog progress("Generating report...", "Cancel", 1, total);
  progress.setWindowModality(Qt::ApplicationModal);
  progress.setMinimumDuration(0);
  progress.setAutoClose(false);
  progress.setAutoReset(false);
  progress.setValue(0);

  QApplication::processEvents();

  vtkVgVideoFrameData frame;

  vtkSmartPointer<vtkMatrix4x4> trackModelToImage =
    vtkSmartPointer<vtkMatrix4x4>::New();

  int count = 0;
  int currId = 1;
  foreach (vtkVgVideoNode* node, nodes)
    {
    if (!node->GetIsStarResult())
      {
      continue;
      }

    vtkVgVideoModel0* vm = node->GetVideoRepresentation()->GetVideoModel();
    vtkVgEventModel* em = vm->GetEventModel();

    if (vqArchiveVideoSource* src =
          vqArchiveVideoSource::SafeDownCast(vm->GetVideoSource()))
      {
      video->DeepCopy(*src, vqArchiveVideoSource::CopyClipShared);
      video->SetLooping(0);
      }
    else
      {
      qDebug() << "Unknown video source.";
      continue;
      }

    vtkVgEvent* event = em->GetEvent(node->GetInstanceId());
    if (!event)
      {
      qDebug() << "No event found.";
      continue;
      }

    writer.setEvent(event, currId++, node->GetNote(), 0,
                    node->GetRank(), node->GetRelevancyScore(),
                    node->GetMissionId(), node->GetStreamId());

    vtkVgTimeStamp start = event->GetStartFrame();
    vtkVgTimeStamp end = event->GetEndFrame();

    // Compute timestamp halfway through the event
    vtkVgTimeStamp mid(0.5 * (start.GetTime() + end.GetTime()));
    if (start.HasFrameNumber() && end.HasFrameNumber())
      {
      unsigned frame = (start.GetFrameNumber() + end.GetFrameNumber()) / 2;
      mid.SetFrameNumber(frame);
      }

    // Set timestamp to the next closest frame with a valid region
    vtkIdType npts;
    vtkIdType* ids;
    vtkVgTimeStamp prev =  mid;
    if (!event->GetRegionAtOrAfter(mid, npts, ids) && mid == prev)
      {
      qDebug() << "Could not get event midpoint region.";
      continue;
      }

    if (video->GetFrame(&frame, mid.GetTime()) == VTK_OK)
      {
      writer.setImageData(frame.VideoImage,
                          frame.TimeStamp, 0,
                          frame.VideoMatrix);

      vtkVgTrack* track =
        this->TrackNode->GetTrackModel()->GetTrack(event->GetId());
      if (track && frame.VideoMatrix)
        {
        // Compute the image-to-context matrix then invert it to get the
        // track-to-image transform (track points are in context space).
        vtkMatrix4x4::Multiply4x4(
          this->TerrainSource->GetCoordinateTransformMatrix(),
          frame.VideoMatrix, trackModelToImage);
        trackModelToImage->Invert();

        writer.setTrack(track, trackModelToImage);
        }
      else
        {
        writer.setTrack(0);
        }

      writer.writeEventSummary();
      }
    else
      {
      qDebug() << "Could not get midpoint frame.";
      continue;
      }

    // If not generating video, we're done
    if (!generateVideo)
      {
      progress.setValue(++count);
      continue;
      }

    // Write video images
    video->GetFrame(&frame, start.GetTime());
    int framenum = 0;
    do
      {
      writer.setImageData(frame.VideoImage,
                          frame.TimeStamp, 0,
                          frame.VideoMatrix);
      writer.writeEventVideoImage(++framenum);
      if (progress.wasCanceled())
        {
        break;
        }
      progress.setValue(++count);
      }
    while (video->GetNextFrame(&frame) == VTK_OK && frame.TimeStamp <= end);

    // Encode video from images
    writer.writeEventVideo();
    }

  writer.writeOverview();

  // Now export KML
  this->exportKml(path);
}

//-----------------------------------------------------------------------------
void vqCore::exportSvm(QString path)
{
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly |
                 QIODevice::Truncate))
    {
    qDebug() << file.errorString();
    QMessageBox::warning(0, QString(), "Unable to write SVM file.");
    return;
    }

  if (this->SavedQueryPlan.isSimilarityQuery())
    {
    auto& query = *this->SavedQueryPlan.constSimilarityQuery();

    const int iqrModelSize = static_cast<int>(query.IqrModel.size());
    if (iqrModelSize)
      {
      const char* rawModel =
        reinterpret_cast<const char*>(&query.IqrModel[0]);
      const QByteArray encodedModel =
        QByteArray::fromRawData(rawModel, iqrModelSize).toBase64();

      file.write(encodedModel);
      }
    }

  file.close();
}

//-----------------------------------------------------------------------------
void vqCore::exportKml(QString path)
{
  QString filePath;
  QFileInfo fileInfo(path);
  if (fileInfo.isDir())
    {
    filePath = QString(fileInfo.canonicalFilePath() + '/' + "viqui.kml");
    }
  else
    {
    filePath = fileInfo.absoluteFilePath();
    }

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly |
                 QIODevice::Text |
                 QIODevice::Truncate))
    {
    qDebug() << file.errorString();
    QMessageBox::warning(0, QString(), "Unable to write KML file.");
    return;
    }

  // Sort by start time
  QList<vtkVgVideoNode*> nodes = this->QueryResultNodes.values();
  qSort(nodes.begin(), nodes.end(), eventEarlierThan);

  vvKmlWriter kmlWriter;

  vtkVgTrackModel* trModel = this->TrackNode->GetTrackModel();
  if (!trModel)
    {
    QString err = QString("Invalid track model.")
                  + QString("Unable to write KML file.");
    QMessageBox::warning(0, QString(), err);
    }

  // Loop over each node, find associated events and finally tracks
  foreach (vtkVgVideoNode* node, nodes)
    {
    if (node->GetIsStarResult())
      {
      vtkVgVideoModel0* vm = node->GetVideoRepresentation()->GetVideoModel();
      vtkVgEventModel* em = vm->GetEventModel();

      if (vtkVgEvent* event = em->GetEvent(node->GetInstanceId()))
        {
        vtkIdType evId = event->GetId();
        vtkVgTrack* track = trModel->GetTrack(evId);
        if (!track)
          {
          continue;
          }

        vtkIdList* ids = track->GetPointIds();
        vtkPoints* points = track->GetPoints();

        double trackColor[3];
        track->GetColor(trackColor);

        vvKmlLine* line = kmlWriter.createLine();
        line->setId(QString("Result_%1").arg(track->GetId()));
        line->setDescription(QString::fromLocal8Bit(node->GetNote()));
        line->setColor(trackColor[0], trackColor[1], trackColor[2], 1.0);

        double loc[3];
        double lat, lon;
        for (int i = 0; i < ids->GetNumberOfIds(); ++i)
          {
          points->GetPoint(ids->GetId(i), loc);
          this->transformContextToLatLon(loc[0], loc[1], lat, lon);

          // Ignoring altitude for now
          line->addPoint(lat, lon, 0.0);
          }
        }
      }
    }

  if (kmlWriter.isEmpty())
    {
    qDebug() << "Nothing to export to KML format.";
    }

  kmlWriter.write(file);
}

//-----------------------------------------------------------------------------
void vqCore::showTrackingClips(QList<vtkVgVideoNode*> nodes,
                               vqTrackingClipViewer* viewer, int limit)
{
  vqTrackingClipViewer::ClipVector clips;
  clips.reserve(nodes.size());

  if (limit == 0)
    {
    limit = nodes.size();
    }

  foreach (vtkVgVideoNode* node, nodes)
    {
    if (limit-- == 0)
      {
      break;
      }

    if (!node->GetHasVideoData())
      {
      continue;
      }

    vtkVgVideoModel0* VM = node->GetVideoRepresentation()->GetVideoModel();
    vtkVgEventModel* EM = VM->GetEventModel();

    // create a copy of the event
    EM->InitEventTraversal();
    vtkVgEvent* event = EM->GetNextEvent().GetEvent();
    vtkVgEvent* clonedEvent = vtkVgEvent::New();
    clonedEvent->DeepCopy(event);

    // we must 'deep copy' the video source
    vqArchiveVideoSource::SmartPtr video =
      vtkSmartPointer<vqArchiveVideoSource>::New();
    if (vqArchiveVideoSource* src =
          vqArchiveVideoSource::SafeDownCast(VM->GetVideoSource()))
      {
      video->DeepCopy(*src, vqArchiveVideoSource::CopyClipDetached);
      }

    // construct the tracking clip
    vtkVQTrackingClip* TC = vtkVQTrackingClip::New();
    TC->SetRank(node->GetRank());
    TC->SetVideo(video);
    TC->SetVideoNode(node);
    TC->SetEvent(clonedEvent);
    clips.push_back(std::make_pair(0, TC));
    clonedEvent->FastDelete();
    TC->FastDelete();
    }

  if (!clips.empty())
    {
    viewer->InsertClips(clips);
    viewer->show();
    }
}

//-----------------------------------------------------------------------------
vqContour* vqCore::createStaticContour(vgGeocodedPoly region, bool finalized)
{
  vtkMatrix4x4* mat = this->TerrainSource->GetCoordinateTransformMatrix();

  vtkIdList* ids = vtkIdList::New();
  vtkPoints* pts = vtkPoints::New();
  vtkPolyData* pd = vtkPolyData::New();
  vtkCellArray* ca = vtkCellArray::New();

  vtkIdType size = static_cast<vtkIdType>(region.Coordinate.size());

  ids->SetNumberOfIds(size + 1);
  pts->SetNumberOfPoints(size);

  // construct a polydata
  for (vtkIdType i = 0; i < size; ++i)
    {
    double pt[] =
      {
      region.Coordinate[i].Longitude,
      region.Coordinate[i].Latitude,
      0.0,
      1.0
      };

    // lat-lon -> world
    vtkVgApplyHomography(pt, mat, pt);
    pt[2] = 0.0;
    pts->SetPoint(i, pt);
    ids->SetId(i, i);
    }
  ids->SetId(size, 0);

  ca->InsertNextCell(ids);
  pd->SetPoints(pts);
  pd->SetLines(ca);

  // create contour
  vqContour* contour =
    new vqContour(this->ContextViewer->GetRenderWindowInteractor());

  contour->Initialize(pd);
  contour->AddToScene(this->ContextSceneRoot);

  if (finalized)
    {
    contour->SetVisible(this->QueryRegionVisible);
    contour->Finalize();
    }

  contour->End();

  ids->FastDelete();
  pts->FastDelete();
  pd->FastDelete();
  ca->FastDelete();

  return contour;
}

//-----------------------------------------------------------------------------
bool vqCore::filterSpatially(vqArchiveVideoSource* arVideoSource,
                             const std::vector<vvDescriptor>& descriptors)
{
  if (!this->SpatialFilterActive)
    {
    return true;
    }

  bool resultSelected = false;

  // Get the clip meta data
  const vgKwaVideoClip* videoClip = arVideoSource->GetCurrentVideoClip();
  if (!videoClip)
    {
    std::cerr << "Corresponding video clip not found. Returning." << std::endl;
    return false;
    }
  const vgKwaVideoClip::MetadataMap clipMetaData = videoClip->metadata();

  vtkMatrix4x4* latLonToContext =
    this->TerrainSource->GetCoordinateTransformMatrix();

  vtkSmartPointer<vtkMatrix4x4> imageToContext(
    vtkSmartPointer<vtkMatrix4x4>::New());

  vtkSmartPointer<vtkPoints> trackPoints;
  bool checkForTripEvents = false;
  if (this->FilterOnEnterTripEvents || this->FilterOnExitTripEvents)
    {
    checkForTripEvents = true;
    trackPoints = vtkSmartPointer<vtkPoints>::New();
    // 60+ second track (at 30fps); should be enough for vast majority of descriptors
    trackPoints->Allocate(2000);
    }

  std::vector<vvDescriptor>::const_iterator iter;
  for (iter = descriptors.begin(); iter != descriptors.end(); iter++)
    {
    vtkSmartPointer<vtkVgTrack> tempTrack;
    if (checkForTripEvents)
      {
      tempTrack = vtkSmartPointer<vtkVgTrack>::New();
      tempTrack->SetPoints(trackPoints);
      trackPoints->Reset();
      }

    vvDescriptorRegionMap::const_iterator regionIter;
    for (regionIter = iter->Region.begin();
         !resultSelected && regionIter != iter->Region.end(); regionIter++)
      {
      vgKwaVideoClip::MetadataMap::const_iterator hi =
        clipMetaData.find(regionIter->TimeStamp);

      if (hi == clipMetaData.end())
        {
        continue;
        }

      vtkSmartPointer<vtkMatrix4x4> imageToLatLonHomographyMatrix;
      if (!computeImageToLatLon(imageToLatLonHomographyMatrix, *hi))
        {
        qDebug() << "Homography matrix computation failed;"
                 " frame ignored for spatial filtering";
        continue;
        }

      vtkMatrix4x4::Multiply4x4(latLonToContext,
                                imageToLatLonHomographyMatrix, imageToContext);

      double yDim = hi->imageSize().height();
      if (yDim <= 0)
        {
        // Image dimensions not available; use 'most likely' guess
        yDim = 480.0;
        }

      if (!checkForTripEvents)
        {
        // clockwise from top left
        double regionPtTransformed[4], regionCornerPts[4][4];
        regionCornerPts[0][0] = regionIter->ImageRegion.TopLeft.X;
        regionCornerPts[1][0] = regionIter->ImageRegion.BottomRight.X;
        regionCornerPts[2][0] = regionIter->ImageRegion.BottomRight.X;
        regionCornerPts[3][0] = regionIter->ImageRegion.TopLeft.X;
        regionCornerPts[0][1] = yDim - regionIter->ImageRegion.TopLeft.Y;
        regionCornerPts[1][1] = yDim - regionIter->ImageRegion.TopLeft.Y;
        regionCornerPts[2][1] = yDim - regionIter->ImageRegion.BottomRight.Y;
        regionCornerPts[3][1] = yDim - regionIter->ImageRegion.BottomRight.Y;

        for (int i = 0; i < 4; i++)
          {
          regionCornerPts[i][2] = 0.0;
          regionCornerPts[i][3] = 1.0;
          imageToContext->MultiplyPoint(regionCornerPts[i], regionPtTransformed);
          if (regionPtTransformed[3])
            {
            regionPtTransformed[0] /= regionPtTransformed[3];
            regionPtTransformed[1] /= regionPtTransformed[3];
            regionPtTransformed[2] /= regionPtTransformed[3];
            if (this->QueryResultSelector->EvaluatePoint(regionPtTransformed))
              {
              resultSelected = true;
              break;
              }
            }
          }
        }
      else
        {
        double trackPtTransformed[4], bottomCenterRegionPt[4] =
          {
          (regionIter->ImageRegion.BottomRight.X + regionIter->ImageRegion.TopLeft.X) / 2.0,
          yDim - regionIter->ImageRegion.BottomRight.Y - 1,
          0.0,
          1.0
          };
        imageToContext->MultiplyPoint(bottomCenterRegionPt, trackPtTransformed);
        if (trackPtTransformed[3])
          {
          trackPtTransformed[0] /= trackPtTransformed[3];
          trackPtTransformed[1] /= trackPtTransformed[3];
          tempTrack->InsertNextPoint(regionIter->TimeStamp, trackPtTransformed,
                                     vtkVgGeoCoord());
          }
        }
      }
    if (checkForTripEvents)
      {
      bool myCheckForTripEvent = false, myCheckForEnterEvent = this->FilterOnEnterTripEvents,
           myCheckForExitEvent = this->FilterOnExitTripEvents;
      this->QueryTripWireManager->CheckTrackForTripWireEvents(tempTrack, myCheckForTripEvent,
          myCheckForEnterEvent, myCheckForExitEvent);
      if (myCheckForTripEvent || myCheckForEnterEvent || myCheckForExitEvent)
        {
        resultSelected = true;
        }
      }
    }

  return resultSelected;
}

//-----------------------------------------------------------------------------
bool vqCore::resultPassesFilters(vqCore::ResultId iid) const
{
  CHECK_ARG(this->QueryResults.contains(iid), false);

  const vvQueryResult& result = this->QueryResults[iid];
  return (result.RelevancyScore >= this->ResultFilter.Threshold);
}

//-----------------------------------------------------------------------------
vqResultFilter vqCore::resultFilter() const
{
  return this->ResultFilter;
}

//-----------------------------------------------------------------------------
void vqCore::setResultFilter(vqResultFilter filter)
{
  this->ResultFilter = filter;
  if (!this->QueryResults.isEmpty())
    {
    vqScopedOverrideCursor busy;

    QList<ResultId> scoringRequestIds = this->ScoringRequestNodes.keys();

    // Clear and recalculate result display list
    this->displayResults(vqSettings().resultPageCount());

    qtUtil::mapBound(scoringRequestIds, this,
                     &vqCore::createScoringRequestNode);

    // Update scene
    this->updateScene();

    // Must notify that results have changed, due to scoring-request list being
    // recreated (after updateScene, so colors have been recalculated)
    emit this->resultsUpdated();
    }
}

//-----------------------------------------------------------------------------
void vqCore::showBestClips(vqTrackingClipViewer* viewer)
{
  QList<vtkVgVideoNode*> queryResults = this->queryResultNodes();
  if (!queryResults.empty())
    {
    this->showTrackingClips(queryResults, viewer, 10);
    }
}

//-----------------------------------------------------------------------------
void vqCore::showNextTrackingClips(vtkIdType previousId, int count,
                                   vqTrackingClipViewer* viewer)
{
  // really want to get the next "count" nodes given the current sorting, but
  // I'm not sure how to get that from the tree.  Thus, for now, get the next
  // next items assuming rank order (and that they're visible in the tree)
  QList<vtkVgVideoNode*> rankSortedQueryResults = this->queryResultNodes();

  QList<vtkVgVideoNode*> subList;
  QList<vtkVgNodeBase*> nodeBaseList;

  bool previousFound = false;
  foreach (vtkVgVideoNode* node, rankSortedQueryResults)
    {
    if (!previousFound)
      {
      if (node->GetInstanceId() == previousId)
        {
        previousFound = true;
        }
      continue;
      }

    // ony add those that have video data
    if (!node->GetHasVideoData())
      {
      continue;
      }

    if (count-- == 0)
      {
      break;
      }

    subList.push_back(node);
    nodeBaseList.push_back(node);
    }

  // if we founnd additional/next items to show, update the track viewer
  if (!subList.empty())
    {
    viewer->RemoveAllClips();
    this->selectNodes(nodeBaseList);
    this->showTrackingClips(subList, viewer);
    }
}

//-----------------------------------------------------------------------------
void vqCore::setShowQueryRegion(bool enable)
{
  this->QueryRegionVisible = enable;
  if (this->QueryRegionContour)
    {
    this->QueryRegionContour->SetVisible(enable);
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vqCore::setShowTracksOnContext(bool enable)
{
  if (this->ShowTracksOnContext != enable)
    {
    this->ShowTracksOnContext = enable;

    this->TrackNode->GetTrackRepresentation()->SetVisible(enable ? 1 : 0);
    this->TrackNode->GetTrackRepresentation()->Update();
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vqCore::setGroundTruthEventType(int type)
{
  if (type != this->GroundTruthEventType)
    {
    this->GroundTruthEventType = type;

    // refresh results
    this->ReceivingGroundTruthResults = false;
    this->resetQueryResults(false, false);
    this->queryFinished(false); // hack...
    }
}

//-----------------------------------------------------------------------------
void vqCore::setShowVideoClipsOnContext(bool enable)
{
  this->ShowVideoClipsOnContext = enable;

  if (!this->ContextVideoRoot)
    {
    return;
    }

  vqVideoNodeVisitor visitor;
  visitor.VideoClipVisible = enable;
  visitor.UpdateVideoClipVisibility = true;
  visitor.UpdateVideoOutlineVisibility = false;

  visitor.Visit(*this->ContextVideoRoot.GetPointer());

  this->update();
}

//-----------------------------------------------------------------------------
void vqCore::setShowVideoOutlinesOnContext(bool enable)
{
  this->ShowVideoOutlinesOnContext = enable;

  if (!this->ContextVideoRoot)
    {
    return;
    }

  vqVideoNodeVisitor visitor;
  visitor.VideoOutlineVisible = enable;
  visitor.UpdateVideoClipVisibility = false;
  visitor.UpdateVideoOutlineVisibility = true;

  visitor.Visit(*this->ContextVideoRoot.GetPointer());

  this->update();
}

//-----------------------------------------------------------------------------
void vqCore::addTrackToContextTrackModel(vtkVgVideoNode* node)
{
  vtkVgEventModel* eventModel = node->GetVideoRepresentation()->
                                GetEventRepresentation()->GetEventModel();

  if (eventModel->GetNumberOfEvents() != 1)
    {
    return;
    }

  vtkPoints* points = eventModel->GetSharedRegionPoints();
  eventModel->InitEventTraversal();
  vtkVgEventBase* theEvent = eventModel->GetNextEvent().GetEvent();

  // See if track already in the model.
  if (this->TrackNode->GetTrackModel()->GetTrack(theEvent->GetId()))
    {
    return;
    }

  // Basic setup of track, and add to model.
  vtkVgTrack* newTrack = vtkVgTrack::New();
  newTrack->SetPoints(this->TrackNode->GetTrackModel()->GetPoints());
  newTrack->SetId(theEvent->GetId());

  this->TrackNode->GetTrackModel()->AddTrack(newTrack);
  newTrack->FastDelete();

  vqArchiveVideoSource* arVideoSource = vqArchiveVideoSource::SafeDownCast(
                                          node->GetVideoRepresentation()->GetVideoModel()->GetVideoSource());

  const vgKwaVideoClip* videoClip = arVideoSource->GetCurrentVideoClip();
  if (!videoClip)
    {
    std::cerr << "Corresponding video clip not found. Returning." << std::endl;
    return;
    }
  const vgKwaVideoClip::MetadataMap clipMetaData = videoClip->metadata();

  vtkMatrix4x4* latLonToContext =
    this->TerrainSource->GetCoordinateTransformMatrix();
  vtkSmartPointer<vtkMatrix4x4> imageToContext(
    vtkSmartPointer<vtkMatrix4x4>::New());

  // now add the points making up the track
  vtkVgTimeStamp timeStamp;
  vtkIdType npts, *pts;
  for (theEvent->InitRegionTraversal();
       theEvent->GetNextRegion(timeStamp, npts, pts);)
    {
    const vgTimeStamp ts = timeStamp.GetRawTimeStamp();
    if (!clipMetaData.contains(ts))
      {
      continue;
      }

    // Know we add region starting from top left, and moving clockwise,
    // thus center of region bottom is x average of 3rd and 4th points
    double pt0[3], pt1[3];
    points->GetPoint(pts[2], pt0);
    points->GetPoint(pts[3], pt1);

    // Now transform from image to context coordinates
    vtkSmartPointer<vtkMatrix4x4> imageToLatLonHomographyMatrix;
    if (!computeImageToLatLon(imageToLatLonHomographyMatrix,
                              clipMetaData[ts]))
      {
      qDebug() << "Homography matrix computation failed, track point not added!";
      continue;
      }

    vtkMatrix4x4::Multiply4x4(latLonToContext,
                              imageToLatLonHomographyMatrix, imageToContext);

    double tp[2];
    vtkVgApplyHomography(0.5 * (pt0[0] + pt1[0]), pt0[1], imageToContext, tp);
    newTrack->InsertNextPoint(timeStamp, tp, vtkVgGeoCoord());
    }
}

//-----------------------------------------------------------------------------
void vqCore::updateTrackVisibility()
{
  vtkVgTrackModel* tm = this->TrackNode->GetTrackModel();
  bool changed = false;

  // Update the visibility of tracks to match the visibility of the
  // corresponding results.
  tm->InitTrackTraversal();
  vtkVgTrackInfo info;
  while ((info = tm->GetNextTrack()).GetTrack())
    {
    vtkIdType id = info.GetTrack()->GetId();

    QHash<ResultId, vtkVgVideoNode*>::iterator itr;
    itr = this->QueryResultNodes.find(id);
    if (itr == this->QueryResultNodes.end())
      {
      itr = this->ScoringRequestNodes.find(id);
      if (itr == this->ScoringRequestNodes.end())
        {
        itr = this->GroundTruthNodes.find(id);
        if (itr == this->GroundTruthNodes.end())
          {
          // Should never get here.
          continue;
          }
        }
      }

    bool nodeVisible = itr.value()->GetVisible();
    if (info.GetDisplayTrack() != nodeVisible)
      {
      changed = true;
      tm->SetTrackDisplayState(id, nodeVisible);
      }
    }

  if (changed)
    {
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vqCore::updateTrackColors()
{
  vtkVgTrackModel* trackModel = this->TrackNode->GetTrackModel();
  trackModel->InitTrackTraversal();
  while (vtkVgTrack* track = trackModel->GetNextTrack().GetTrack())
    {
    vtkVgVideoNode* node = this->getResultNode(track->GetId());
    if (node)
      {
      double color[3];
      this->ItemColorLUT->GetColor(node->GetColorScalar(), color);
      trackModel->SetTrackColor(track->GetId(), color);
      }
    }
}

//-----------------------------------------------------------------------------
void vqCore::lodToggleVisibility(double scaleBoundary, int stateUnder,
                                 bool events, bool markers, bool override)
{
  // \NOTE: factor of 3.5 is hard coded for now.
  double currentViewScale =
    this->ContextViewer->GetCurrentScale() / this->ViewScaleFactor;

  bool toggled = true;

  if (!override)
    {
    toggled = (this->LastEventViewScale > scaleBoundary)
              || (this->LastMarkerViewScale > scaleBoundary);
    }

  if ((currentViewScale <= scaleBoundary) && (toggled))
    {
    this->LastEventViewScale
      = events ? currentViewScale : this->LastEventViewScale;
    this->LastMarkerViewScale
      = markers ? currentViewScale : this->LastMarkerViewScale;

    this->toggleVisibility(this->QueryResultNodes, stateUnder, events, markers);
    this->toggleVisibility(this->ScoringRequestNodes, stateUnder, events, markers);
    }

  if (!override)
    {
    toggled = (this->LastEventViewScale <= scaleBoundary)
              || (this->LastMarkerViewScale <= scaleBoundary);
    }

  if ((currentViewScale > scaleBoundary) && (toggled))
    {
    this->LastEventViewScale
      = events ? currentViewScale : this->LastEventViewScale;
    this->LastMarkerViewScale
      = markers ? currentViewScale : this->LastMarkerViewScale;

    this->toggleVisibility(this->QueryResultNodes, !stateUnder, events, markers);
    this->toggleVisibility(this->ScoringRequestNodes, !stateUnder, events, markers);
    }
}

//-----------------------------------------------------------------------------
void vqCore::toggleVisibility(QHash<ResultId, vtkVgVideoNode*>& nodes,
                              int value, bool events, bool markers)
{
  foreach (vtkVgVideoNode* node, nodes)
    {
    if (vtkVgVideoRepresentation0* videoRepresentation =
          vtkVgVideoRepresentation0::SafeDownCast(node->GetVideoRepresentation()))
      {
      if (events)  { videoRepresentation->SetEventVisible(value); }
      if (markers) { videoRepresentation->SetMarkerVisible(value); }
      }
    }
}
