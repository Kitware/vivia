/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqCore_h
#define __vqCore_h

// Qt includes
#include <QVTKWidget.h>
#include <QBasicTimer>
#include <QString>
#include <QList>
#include <QHash>
#include <QMultiMap>
#include <QSet>
#include <QSharedPointer>
#include <QUrl>

// QtExtensions includes
#include <qtGradient.h>
#include <qtStatusSource.h>

// VTK includes
#include <vtkSmartPointer.h>

// VISGUI includes
#include <vtkVgTimeStamp.h>
#include <vvQueryFormulation.h>
#include <vvQueryInstance.h>
#include <vvQueryResult.h>

// Viqui includes
#include "vqResultFilter.h"
#include "vtkVQBlastLayoutNode.h"

// Forward declarations.
class QLabel;
class QProgressBar;

class qtStatusManager;
class qtStatusNotifier;

class vtkLookupTable;
class vtkPolyData;
class vtkRenderWindow;
class vtkTimerLog;

class vtkVgContourOperatorManager;
class vtkVgGeode;
class vtkVgGroupNode;
class vtkVgInteractorStyleRubberBand2D;
class vtkVgNodeBase;
class vtkVgTrackNode;
class vtkVgTransformNode;
class vtkVgTripWireManager;
class vtkVgViewerBase;
class vtkVgTerrainSource;
class vtkVQTrackingClip;
class vtkVgVideoNode;

class vgKwaArchive;

struct vvQueryResult;

class vqArchiveVideoSource;
class vqContour;
class vqQueryParser;
class vqTrackingClipViewer;
class vtkVgVideoViewer;

class vqCore : public QObject
{
  Q_OBJECT

public:
  typedef long long ResultId;
  typedef long long ResultRank;

  enum LoadQueryFlag
    {
    GroundTruth = 0x1,
    SavePersistent = 0x2
    };
  Q_DECLARE_FLAGS(LoadQueryFlags, LoadQueryFlag)

  enum NodeColorMode
    {
    ColorByRank = 0x0,
    ColorByRelativeScore = 0x1,
    ColorByAbsoluteScore = 0x2
    };

  vqCore();
  virtual ~vqCore();

  void setupUi(QVTKWidget*);

  vtkVgViewerBase* getContextViewer();
  vtkVgInteractorStyleRubberBand2D* getContextInteractorStyle();

  void setVideoProviders(const QList<QUrl>&);
  void setQueryServer(const QUrl&);

  bool isQuerySessionActive() const;

  int resultCount() const;

  QList<vtkVgVideoNode*> queryResultNodes();
  QList<vtkVgVideoNode*> scoringRequestNodes();
  QList<vtkVgVideoNode*> groundTruthNodes();

  vvQueryResult* getResult(ResultId iid);
  vvQueryResult* getGroundTruthResult(ResultId iid);

  vtkVgVideoNode* getResultNode(ResultId iid);

  vtkLookupTable* getLookupTable();

  vqResultFilter resultFilter() const;

  bool canIssueQuery();

  vtkSmartPointer<vtkMatrix4x4> getLatLonToContextMatrix() const;
  vtkSmartPointer<vtkMatrix4x4> getContextToLatLonMatrix() const;

  void getCurrentPointerPosition(int& x, int& y);
  void transformDisplayToLatLon(int x, int y, double& lat, double& lon) const;
  void transformContextToLatLon(double x, double y, double& lat, double& lon) const;

signals:
  void displayResultsReset();
  void resultsUpdated();
  void resultSetComplete(bool haveSession, bool requestScoring);

  void queryPlanAvailabilityChanged(bool isAvailable);
  void queryResultsAvailabilityChanged(bool isAvailable);

  void processingQuery(vvQueryInstance);

  void formulationComplete(QList<vvDescriptor>,
                           QList<vvTrack> = QList<vvTrack>());
  void formulationFailed();

  void regionComplete(vgGeocodedPoly region);

  void FocusChanged(vtkVgVideoNode& node);
  void FocusChanged(vtkVgNodeBase& node);

  void NextInStack(vtkVgNodeBase& node);

  void coloringChanged();
  void updatedResultScoreIndicators();

  void forcedRender();

  void playedOrStopped(vtkVgVideoNode& videoNode);

  void scrolledForward(vtkVgNodeBase& node, bool* eventHandled);
  void scrolledBackward(vtkVgNodeBase& node, bool* eventHandled);

  void selected(QList<vtkVgNodeBase*> nodes);
  void activated(vtkVgNodeBase& node);

  void initialRefinementRequested();

public slots:
  void registerStatusWidget(QLabel*);
  void registerStatusWidget(QProgressBar*);

  void registerStatusSource(qtStatusNotifier*);

  bool loadQuery(QUrl, LoadQueryFlags = 0);
  bool formulateQuery(vvProcessingRequest, bool bypassCache = false,
                      qtStatusManager* statusTarget = 0);
  bool processQuery(vvQueryInstance);
  bool processDatabaseVideoQuery(vvQueryInstance);
  bool requestRefinement(int resultsToScore);
  bool refineQuery();

  void displayResults(int count, int first = 0);
  void displayGroundTruthResults(int count, int first = 0);

  bool findResult(ResultId iid);

  void setResultFilter(vqResultFilter);

  void showBestClips(vqTrackingClipViewer* viewer);
  void showNextTrackingClips(vtkIdType previousId, int count,
                             vqTrackingClipViewer* viewer);

  void saveQueryPlan();
  void saveResults();
  void exportResults(QList<vtkVgVideoNode*> results, QString exporterId);

  void openExternal(QUrl clipUri, QString streamId, double time);

  void generateReport(QString path, bool generateVideo);
  void exportKml(QString path);

  void setUserScore(ResultId iid, int score);

  void setResultNote(ResultId iid, const QString& note);
  void setResultStarred(ResultId iid, bool starred);

  vtkSmartPointer<vtkVgGroupNode> createScene();

  void start();
  void update();
  void postRender();

  void onLeftClick();
  void onRightButtonPress();
  void onRightButtonRelease();
  void onMouseWheelForward();
  void onMouseWheelBackward();
  void onPKeyPress();

  void onContextMenuEvent();

  void addRasterLayer(QUrl uri);

  void drawRegion();
  void setRegion(vgGeocodedPoly region);

  void updateScene();
  void updateSources();
  void updateLOD(bool override = false);
  void updateResultScoreIndicators();
  void updateLayoutStacks();
  void updateLayoutStackBarLocations();
  void updateStackLayoutForVideoPlaying(vtkVgNodeBase& videoNode);
  void updateTrackVisibility();

  void videoStopped(vtkVgNodeBase& videoNode);


  void updateStackLayoutForVideoStopped();
  void onSelectInStackLayoutWidget(vtkObject* caller);

  void activateNode(vtkVgNodeBase& node);
  void selectNodes(QList<vtkVgNodeBase*> nodes);

  void activateResult(ResultId iid);
  void selectResult(ResultId iid);

  void setColorMappingMode(NodeColorMode);
  void setColorShadingMode(qtGradient::InterpolationMode);
  void reloadColorGradient();

  void setColorByRank()
    { this->setColorMappingMode(ColorByRank); }
  void setColorByRelativeScore()
    { this->setColorMappingMode(ColorByRelativeScore); }
  void setColorByAbsoluteScore()
    { this->setColorMappingMode(ColorByAbsoluteScore); }

  void setColorShadingDiscrete()
    { this->setColorShadingMode(qtGradient::InterpolateDiscrete); }
  void setColorShadingLinear()
    { this->setColorShadingMode(qtGradient::InterpolateLinear); }
  void setColorShadingCubic()
    { this->setColorShadingMode(qtGradient::InterpolateCubic); }

  void resetView();
  void resetViewToQueryResults();

  void setViewToExtents(double extents[4]);

  void showTrackingClips(QList<vtkVgVideoNode*> nodes,
                         vqTrackingClipViewer* viewer, int limit = 0);

  void setShowQueryRegion(bool enable);

  void setGroundTruthEventType(int type);

  void setShowVideoClipsOnContext(bool enable);
  void setShowVideoOutlinesOnContext(bool enable);
  void setShowTracksOnContext(bool enable);

protected slots:
  void finalizeQueryLoading();

  void formulationFinished();
  void transferQueryStatus();

  void acceptQueryResult(vvQueryResult queryResult, bool requestScoring);
  void releaseQuery();

  void queryFinished(bool requestScoring);

  void completeRegion();

  void forceRender();

  void renderLoopOn();
  void renderLoopOff();

  void selectItemsOnContext();

protected:
  bool startQuerySession(bool formulate);
  void endQuerySession(bool clearResults);
  void connectParser(vqQueryParser*);

  void addQueryResult(const vvQueryResult& queryResult);
  void addGroundTruthResult(const vvQueryResult& queryResult);

  void resetQueryResults(bool clearResults, bool clearDisplayResults = true);
  void layoutResults();
  vtkSmartPointer<vtkVgNodeBase> addResultNode(const vvQueryResult& queryResult,
                                               vtkVgVideoNode*& node,
                                               bool useSpatialFilter,
                                               bool isGroundTruth = false,
                                               int eventTypeFilter = -1);
  void createScoringRequestNode(ResultId iid);
  void unregisterAllNodes();

  bool updateTerrainSource();

  void colorNodesByRank();
  void colorNodesByRelativeScore();
  void colorNodesByAbsoluteScore();
  void recomputeNodeColors();
  void updateNodeColor(vtkVgVideoNode* node);
  void updateTrackColors();

  void setLutValue(int index, const QColor& color);

  vqContour* createStaticContour(vgGeocodedPoly region, bool finalized);

  bool filterSpatially(vqArchiveVideoSource* arVideoSource,
                       const std::vector<vvDescriptor>& descriptors);
  bool resultPassesFilters(ResultId iid) const;

  void addTrackToContextTrackModel(vtkVgVideoNode* node);

  virtual void timerEvent(QTimerEvent* event);

  void setupTracksOnContext();

  void selectClipsOnContext(int* startScreenPosition, int* endScreenPosition);
  void selectTracksOnContext(int* startScreenPosition, int* endScreenPosition);

  vtkVgNodeBase* findNodeByTrackId(vtkIdType id);

  void CacheDatabaseQueryResults(vvQueryResult result);
  void ExtractTracksAndDescriptorsFromCache();

  vtkVgVideoNode* addResult(ResultId iid, bool useSpatialFilter);

  void lodToggleVisibility(double scaleBoundary, int stateUnder,
                           bool events, bool markers, bool override);
  void toggleVisibility(QHash<ResultId, vtkVgVideoNode*>& nodes,
                        int value, bool events, bool markers);

  double                                            TerrainBounds[6];

  vtkSmartPointer<vtkVgVideoViewer>                 ContextViewer;
  vtkSmartPointer<vtkVgInteractorStyleRubberBand2D> ContextInteractorStyle;

  double                                            ApplicationStartTime;
  vtkSmartPointer<vtkTimerLog>                      TimerLog;
  vtkVgTimeStamp                                    ApplicationTimeStamp;

  vtkSmartPointer<vtkVgTerrainSource>               TerrainSource;

  vtkSmartPointer<vtkVgTransformNode>               ContextSceneRoot;
  vtkSmartPointer<vtkVgTransformNode>               ContextVideoRoot;

  std::vector<vtkVQBlastLayoutNode::SmartPtr>       LayoutNodes;

  vvQueryInstance                                   SavedQueryPlan;
  bool                                              SavePersistentQuery;

  QHash<ResultId, vvQueryResult>                    QueryResults;
  QMultiMap<ResultRank, ResultId>                   QueryScoreMap;

  QSet<ResultId>                                    ScoringRequests;

  QHash<ResultId, vvQueryResult>                    GroundTruthResults;
  QMultiMap<double, ResultId>                       GroundTruthScoreMap;

  QHash<ResultId, vtkVgVideoNode*>                  QueryResultNodes;
  QHash<ResultId, vtkVgVideoNode*>                  ScoringRequestNodes;
  QHash<ResultId, vtkVgVideoNode*>                  GroundTruthNodes;

  vqResultFilter                                    ResultFilter;
  QSet<ResultId>                                    StarResults;

  QList<vtkVgVideoNode*>                            QueryResultNodeList;
  QList<vtkVgVideoNode*>                            ScoringRequestNodeList;
  QList<vtkVgVideoNode*>                            GroundTruthNodeList;

  bool                                              QueryResultNodesDirty;
  bool                                              ScoringRequestNodesDirty;
  bool                                              GroundTruthNodesDirty;

  QScopedPointer<vgKwaArchive>                      VideoArchive;
  QUrl                                              QueryServer;
  QBasicTimer                                       ClickTimer;

  vqQueryParser*                                    ActiveQueryParser;

  double                                            ViewScaleFactor;
  double                                            LastEventViewScale;
  double                                            LastMarkerViewScale;

  NodeColorMode                                     NodeColorMappingMode;
  qtGradient::InterpolationMode                     NodeColorShadingMode;

  vtkSmartPointer<vtkLookupTable>                   ItemColorLUT;

  vqContour*                                         Contour;
  vqContour*                                         QueryRegionContour;

  bool                                              DrawingContour;
  bool                                              QueryRegionVisible;

  vtkVQBlastLayoutNode*                             CachedLayoutNode;

  bool                                              SpatialFilterActive;
  bool                                              FilterOnEnterTripEvents;
  bool                                              FilterOnExitTripEvents;

  bool                                              ProcessingDatabaseQuery;

  int LastClickPosition[2];
  int LastRightClickPosition[2];

  qtStatusManager* StatusManager;
  qtStatusSource InteractionStatusSource;

  QObject QuerySessionDummyObject;
  qtStatusSource QuerySessionStatusSource;

  QVTKWidget* RenderWidget;

  vtkSmartPointer<vtkVgContourOperatorManager>      QueryResultSelector;
  vtkSmartPointer<vtkVgTripWireManager>             QueryTripWireManager;

  int                                               UpdateIntervalMSec;
  QTimer*                                           UpdateTimer;

  bool ReceivingGroundTruthResults;
  int  GroundTruthEventType;

  bool RenderPending;

  bool ContinuousRender;

  vtkSmartPointer<vtkVgTrackNode>                   TrackNode;

  bool ShowVideoClipsOnContext;
  bool ShowVideoOutlinesOnContext;
  bool ShowTracksOnContext;

  int SkipInitialDisplayCounter;

  vtkSmartPointer<vtkVgContourOperatorManager>     ContextTracksSelector;

  QList<vvQueryResult>                           DatabaseQueryResultsCache;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vqCore::LoadQueryFlags)

#endif
