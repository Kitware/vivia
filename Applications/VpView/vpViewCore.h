/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpViewCore_h
#define __vpViewCore_h

#include "vpTrackIO.h"

#include <vtkVgTimeStamp.h>   // Required for vtkVgTimeStamp.

#include <vgAttributeSet.h>
#include <vgNamespace.h>

#include <vtkSmartPointer.h>   // Required for smart pointer internal ivars.

#include <QObject>
#include <QSet>
#include <QSharedPointer>
#include <QStringList>

#include <map>
#include <memory>
#include <vector>

namespace kwiver
{
namespace vital
{
class track;
}
}

class vtkActor;
class vtkActor2D;
class vtkCamera;
class vtkEventQtSlotConnect;
class vtkHoverWidget;
class vtkIdList;
class vtkImageActor;
class vtkImageData;
class vtkLegendBoxActor;
class vtkMatrix4x4;
class vtkObject;
class vtkPNGWriter;
class vtkPoints;
class vtkPolyData;
class vtkRenderWindow;
class vtkRenderer;
class vtkTimerLog;
class vtkWindowToImageFilter;

class vtkVgActivity;
class vtkVgActivityTypeRegistry;
class vtkVgBaseImageSource;
class vtkVgContourOperatorManager;
class vtkVgEvent;
class vtkVgEventFilter;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;
class vtkVgGeoCoord;
class vtkVgGraphModel;
class vtkVgGraphRepresentation;
class vtkVgIconManager;
class vtkVgInteractorStyleRubberBand2D;
class vtkVgRepresentationBase;
class vtkVgTemporalFilters;
class vtkVgTrack;
class vtkVgTrackFilter;
class vtkVgTrackTypeRegistry;

class QVTKWidget;
class QComboBox;
class QDockWidget;
class QFileSystemWatcher;
class QProcess;
class QToolBar;

class vtkVpInteractionCallback;
class vtkVpVideoPlayCallback;

class vgConfigParser;
class vgMixerWidget;

class vpActivityConfig;
class vpAnnotation;
class vpBoundingRegion;
class vpBox;
class vpContour;
class vpDatabaseQueryDialog;
class vpEntityConfigWidget;
class vpEventConfig;
class vpFileDataSource;
class vpFrame;
class vpFrameMap;
class vpInformaticsDialog;
class vpModelIO;
class vpNormalcyMaps;
class vpObjectInfoPanel;
class vpProject;
class vpProjectParser;
class vpQtViewer3dWidget;
class vpSessionView;
class vpTimelineDialog;
class vpTrackConfig;
class vpVideoAnimation;

class vtkVpTrackModel;

class vpViewCore : public QObject
{
  Q_OBJECT

public:
  enum enumDisplayModes
    {
    OverViewMode,
    ZoomEventMode,
    ZoomTrackMode
    };

  enum enumEventExpirationMode
    {
    ShowUntilEventEnd,
    ShowUntilTrackEnd
    };

  enum enumAnnotationMode
    {
    AM_None,
    AM_Track,
    AM_SingleFrameTrack,
    AM_SceneElement,
    };

  // Constructor/Destructor
  vpViewCore();
  virtual ~vpViewCore();

  void cleanUp();

  void setupRenderWidget(QVTKWidget* renderWidget);
  void setupTrackFilters(vgMixerWidget* filterWidget);
  void setupEventFilters(vgMixerWidget* filterWidget, QComboBox* presetWidget);
  void setupActivityFilters(vgMixerWidget* filterWidget);
  void setupNormalcyMapsFilters(vgMixerWidget* filterWidget);
  void setupTrackAttributeColors();

  void setSessionView(vpSessionView* view);
  void setupEventConfig(vpEntityConfigWidget* configWidget);
  void setupActivityConfig(vpEntityConfigWidget* configWidget);

  inline void setTrackOffset(int* trackOffset);

  void setOverviewDisplay(vpProject* project);

  void addTrackFilter(vgMixerWidget* filterWidget, int typeId,
                      const QString& typeName);

  // Set/Get functions.
  inline int    getNumberOfFrames();
  inline int    getMinimumFrameNumber();
  inline int    getMaximumFrameNumber();
  inline int    getCurrentFrameNumber();
  inline vtkVgTimeStamp getCoreTimeStamp();
  inline vtkVgTimeStamp getImageryTimeStamp();

  // Note: This is a low level function that should almost never be called
  // externally. Use the frame number or time-based functions instead.
  inline unsigned int getCurrentFrameIndex();

  double* getCurrentFrameColorScalarRange();
  double* getFrameColorScalarRange(int frameNumber);

  double getMinimumTime();
  double getMaximumTime();
  double getCurrentTime();

  void setCurrentTime(const vtkVgTimeStamp& timeStamp);

  // Unlike getCurrentTime(), when using timestamp data without time-based
  // indexing, this will return the actual timestamp time of the frame. This
  // should be used for display purposes only.
  double getCurrentFrameTime();

  void setFrameNumberOffset(int offset);
  int getFrameNumberOffset() { return this->FrameNumberOffset; }

  void setObjectExpirationTime(const vtkVgTimeStamp& time);
  inline vtkVgTimeStamp getObjectExpirationTime();

  void setIconSize(int size, bool render);
  void setEventExpirationMode(enumEventExpirationMode mode, bool render);

  inline double getRequestFPS() const;

  vtkRenderer* getSceneRenderer();

  vtkVgInteractorStyleRubberBand2D* getInteractorStyle();

  void moveCameraTo(double pos[2], bool forceRender = true);

  void refreshSelectionPanel(bool rebuild = false);

  // Update and render the scene. Use this method if there have not been
  // any changes that would be visible through the ui views, such as a
  // change in filtering.
  void updateScene();

  // Update and render the scene and object views
  void update();

  void updateExtents();

  void render(bool logTime = true);

  void setOverviewDisplayState(bool state);

  void exitAdjudicationMode();

  void setDisplayAOIOutlineState(bool flag = true);

  void focusBounds(double bounds[4]);
  void focusTimeInterval(const vtkVgTimeStamp& start,
                         const vtkVgTimeStamp& end);

  void focusTrack(int sessionId, vtkIdType trackId,
                  vtkVgTimeStamp start = vtkVgTimeStamp(),
                  vtkVgTimeStamp end = vtkVgTimeStamp());

  void focusEvent(int sessionId, vtkIdType eventId);
  void focusActivity(int sessionId, vtkIdType activityId);

  void doZoomToExtent(double extents[4]);
  void getImageSpaceViewportExtents(int (&extents)[4]);

  void showAnnotation(int sessionId, int objectType, vtkIdType objectId);
  void hideAnnotation();

  int  getCreateTrackId(int session);
  void setCreateTrackId(int id, int session);

  void setSingleFrameAnnotation(bool state);
  bool createTrack(int trackId, int session, bool isFse = false);
  int  createEvent(int type, vtkIdList* ids, int session);

  int getTrackTypeIndex(const char* typeName);
  void removeUnusedTrackTypes(const QSet<QString>& typesToKeep = {});

  void updateTrack(vtkVgTrack*, const std::shared_ptr<kwiver::vital::track>&,
                   const QMap<int, vgTimeStamp>& timeMap,
                   double videoHeight, bool updateToc = false);

  void improveTrack(int trackId, int session);
  bool splitTrack(int trackId, int newTrackId, int session);
  bool mergeTracks(int trackA, int trackB, int session);
  vtkVgTrack* cloneTrack(int trackId, int newTrackId, int session);

  vtkVgEvent* cloneEvent(int eventId, int session);

  void setIdOfTrackToFollow(vtkIdType trackId);
  vtkIdType getIdOfTrackToFollow() { return this->IdOfTrackToFollow; }

  void setSaveRenderedImages(bool state, QString* outputDir = 0);

  bool setAntialiasing(bool);
  bool getAntialiasing() const;

  void setImageFiltering(bool);
  bool getImageFiltering() const;

  void setup3dWidget(vpQtViewer3dWidget* viewer3dWidget);

  int getNumberOfActivities();
  int getNumberOfEvents();
  int getNumberOfTracks();

  int addTemporalFilter(int type,
                        const vtkVgTimeStamp& start, const vtkVgTimeStamp& end);

  void updateTemporalFilterStart(int id, const vtkVgTimeStamp& newStart);
  void updateTemporalFilterEnd(int id, const vtkVgTimeStamp& newEnd);
  void updateTemporalFilterType(int id, int type);
  void enableTemporalFilter(int id, bool newState);

  void removeTemporalFilter(int id);
  void removeAllTemporalFilters();

  bool      isEditingTrack() { return this->TrackEditProjectId != -1; }
  vtkIdType editingTrackId() { return this->EditingTrackId; }

  void mouseLeftViewport();
  void mouseEnteredViewport();

  void setTrackHead(vtkPoints* points);
  void setTrackHeadAndAdvance(vtkPoints* points);

  // Helper functions for converting a point from display to image space.
  // Returns true if the point is within the bound of the image or AOI.
  bool displayToImage(int in[2], int out[2]);
  bool displayToAOI(int in[2], int out[2]);
  bool displayToGeo(int in[2], double& northing, double& easting);

  // Get the distance across a single pixel
  bool getGsd(int displayPoint[2], double& latDist, double& lonDist,
              double& width, double& height);

  // Get average GSD across the current image
  bool getGsd(double& latDistPerPixel, double& lonDistPerPixel,
              double& widthPerPixel, double& heightPerPixel);

  vtkVgBaseImageSource* getImageSource();
  void setImageSourceLevelOfDetailFactor(double factor);
  bool hasMultiLevelOfDetailSource();

  int sessionCount() const;
  bool isSessionEnabled(int session) const;

  vtkVpTrackModel* getTrackModel(int session);
  vtkVgEventModel* getEventModel(int session);

  vtkVgTrackFilter* getTrackFilter();
  vtkVgEventFilter* getEventFilter();

  inline const vtkVgEventTypeRegistry* getEventTypeRegistry() const
    {
    return this->EventTypeRegistry;
    }

  inline vtkVgEventTypeRegistry* getEventTypeRegistry()
    {
    return this->EventTypeRegistry;
    }

  inline const vtkVgTrackTypeRegistry* getTrackTypeRegistry() const
    {
    return this->TrackTypeRegistry;
    }

  inline vtkVgTrackTypeRegistry* getTrackTypeRegistry()
    {
    return this->TrackTypeRegistry;
    }

  void setTrackTrailLength(const vtkVgTimeStamp& duration);
  vtkVgTimeStamp getTrackTrailLength();

  void setUseTimeStampDataIfAvailable(bool state)
    {
    this->UseTimeStampDataIfAvailable = state;
    }

  bool getUsingTimeStampData()
    {
    return this->UsingTimeStampData;
    }

  void setUseTimeBasedIndexing(bool state);

  bool getUsingTimeBasedIndexing()
    {
    return this->UsingTimeBasedIndexing;
    }

  void setEnableWorldDisplayIfAvailable(bool state);
  void setEnableTranslateImage(bool state);

  void setUseZeroBasedFrameNumbers(bool enable)
    {
    this->UseZeroBasedFrameNumbers = enable;
    }

  bool getUseZeroBasedFrameNumbers()
    {
    return this->UseZeroBasedFrameNumbers;
    }

  void setRightClickToEditEnabled(bool enable)
    {
    this->RightClickToEditEnabled = enable;
    }

  void setAutoAdvanceDuringCreation(bool enable)
    {
    this->AutoAdvanceDuringCreation = enable;
    }

  bool getRightClickToEditEnabled()
    {
    return this->RightClickToEditEnabled;
    }

  bool getAutoAdvanceDuringCreation()
    {
    return this->AutoAdvanceDuringCreation;
    }

  void setSceneElementLineWidth(double lineWidth);
  double getSceneElementLineWidth()
    {
    return this->SceneElementLineWidth;
    }
  
  double getColorWindowWidth();
  void setColorWindowWidth(double width, bool renderNow);

  double getColorWindowCenter();
  void setColorWindowCenter(double center, bool renderNow);

  bool isPlaying()
    {
    return this->Playing;
    }

  void setGraphRenderingEnabled(bool enable);

  void updateColorofTracksOfType(int typeIndex, double *rgb);

  void startExternalProcess(QString program, QStringList fields,
                            QString ioPath);
  void startExternalProcess();
  bool isExternalProcessRunning();

  void executeEmbeddedPipeline(int session, const QString& pipelinePath);

  void toWindowCoordinates(double&x, double&y);
  void toWindowCoordinates(double (&xy)[2]);

  void toGraphicsCoordinates(double&x, double&y);
  void toGraphicsCoordinates(double (&xy)[2]);

public slots:
  void newProject();
  void openProject();
  void importProject();

  bool importTracksFromFile(vpProject* project);
  bool importEventsFromFile(vpProject* project, bool tracksImportSuccessful);
  bool importEventLinksFromFile(vpProject* project, bool eventsImportSuccessful);
  bool importActivitiesFromFile(vpProject* project, bool eventsImportSuccessful);
  bool importSceneElementsFromFile(vpProject* project);
  bool importIconsFromFile(vpProject* project);
  bool importOverviewFromFile(vpProject* project);
  bool importNormalcyMapsFromFile(vpProject* project);

  void exportTracksToFile(bool filtered = false);
  void exportEventsToFile();
  void exportSceneElementsToFile();
  void exportImageTimeStampsToFile();

  void onLeftPress();
  void onLeftRelease();
  void onLeftClick();
  void onRightClick();
  void onMouseMove();
  void onSelectionComplete();

  void onEditTrackHeadRegion();

  void resetToAOIView();
  void resetView();
  void resetToViewExtents();
  void getAOIExtents(double extents[4]);

  void decreaseTrackHeadSize();
  void increaseTrackHeadSize();

  int pickScene();

  void CreateInformaticsDisplay(int x, int y, vtkImageData* normalcy);
  void CreateBlendedImageDisplay(int x, int y, vtkImageData* normalcy);

  void setProjectVisible(int index, bool visible);

  void onViewTracks(bool state);
  void onViewTrackHeads(bool state);
  void onViewEvents(bool state);
  void onViewActivities(bool state);
  void onViewSceneElements(bool state);
  void onViewEventIcons(bool show);

  void onShowObjectInfo(int sessionId, vpObjectInfoPanel* objectInfo);

  void onRandomEventColor(bool state);
  void onRandomTrackColor(bool state);
  void onDisplayFullVolume(bool state, bool render = true);

  void SetTrackTypeDisplayState(int trackType, bool state);
  void SetTrackTypeProbabilityLimit(int trackType, double prob);

  void SetEventTypeDisplayState(int eventType, bool state);
  void SetEventInverseDisplayState(int eventType, bool state);
  void SetEventTypeNormalcyThreshold(int eventType, double threshold);
  void UpdateEventDisplayStates(bool state);

  void onUseNormalcyCues(bool enable);
  void onSwapNormalcyCues();

  void SetActivityTypeDisplayState(int type, bool state);
  void SetActivityTypeSaliencyThreshold(int type, double threshold);

  void onIncreaseIconXOffset();
  void onDecreaseIconXOffset();
  void onIncreaseIconYOffset();
  void onDecreaseIconYOffset();

  void onIncreaseOverlayTransparency();
  void onDecreaseOverlayTransparency();

  void onIncreaseSceneElementTransparency();
  void onDecreaseSceneElementTransparency();

  void onPlay();
  void onPause();
  void setLoop(bool state);

  void nextFrame();
  void prevFrame();
  void seekToFrame(const vtkVgTimeStamp& position, vg::SeekMode direction);

  void setPlaybackRate(double rate);
  double getPlaybackRate();

  void onResize(int width, int height);

  void setRequestFPS(double fps);

  void showHideEventLegend(bool show, bool render);
  void showHideTimelineDialog(vpTimelineDialog* dialog, bool show);

  void showHideObjectTags(bool show);

  vpProject* loadProject(const char* fileName);
  vpProject* loadProject(const QSharedPointer<vpModelIO>& modelIO,
                         const std::string& stream,
                         const std::string& name = std::string());
  vpProject* processProject(QScopedPointer<vpProject>& project);
  void closeProject(int sessionId);

  void loadConfig(const char* fileName);

  void writeSpatialFilter(vtkPoints* contourPoints, vtkPoints* filterPoints,
                          unsigned int frameNumber, double time,
                          vtkMatrix4x4* worldtoImageMatrix,
                          const std::string& name, std::ostream& out);
  void writeTemporalFilter(int type, double start, double end,
                           const std::string& name, std::ostream& out);
  void loadFilters();
  void loadFilters(const char* fileName);

  void exportForWeb(const char* path, int paddingFrames);

  void initializeAllOthers();
  void initializeData();
  void initializeDisplay();
  void initializeExtentsBounds();
  void initializeScene();
  void initializeViewInteractions();
  void initializeSources();
  void reinitialize();

  int loadTracks(vpProject* project);
  int loadTrackTraits(vpProject* project);
  int loadEvents(vpProject* project);
  int loadEventLinks(vpProject* project);
  int loadActivities(vpProject* project);
  int loadIcons(vpProject* project);
  int loadSceneElements(vpProject* project);

  int updateTracks();

  void dataChanged();

  void refreshView();

  void reset();

  void onHoverStart();
  void onHoverStop();
  void onHoverTimer();

  void eventTypeChanged(int type);
  void activityTypeChanged(int type);

  void addFilterRegion(const std::string& name,
                       const std::vector<double>& points,
                       vtkMatrix4x4* worldToImageMatrix,
                       vtkVgTimeStamp& timeStamp);

  void drawFilterRegion();
  void completeFilterRegion();

  void setContourVisible(vtkPoints* contourPoints, bool visible);
  void setFilterRegionEnabled(vtkPoints* filterPoints, bool enabled);

  void removeFilterRegion(vtkPoints* contourPoints, vtkPoints* filterPoints);
  void removeAllFilterRegions();

  void saveCameraPosition();
  void restoreCameraPosition();

  void beginEditingTrack(enumAnnotationMode annotationMode, int trackId);
  void stopEditingTrack(enumAnnotationMode annotationMode = AM_None,
                        bool autoremove = true);
  void deleteTrack(int trackId, int session);
  void deleteEvent(int eventId, int session);
  void deleteTrackPoint();

  void setEventSelection(const QList<vtkIdType>& ids, int session);
  void setTrackSelection(const QList<vtkIdType>& ids, int session);

  void setRegionEditMode(int mode);
  void setExternalExecuteMode(int mode);

  void setTrackColorMode(int mode, const QString& attributeGroup = QString());
  int  getTrackColorMode()         { return this->CurrentTrackColorMode; }
  QString getTrackAttributeGroup() { return this->CurrentTrackAttributeGroup; }

  void previousTrackAttributeGroup();
  void nextTrackAttributeGroup();

  vgAttributeSet* getTrackAttributes() { return &this->TrackAttributes; }

  void setStreamingEnabled(bool enable);

  void setTrackUpdateChunkSize(int frames)
    {
    this->TrackUpdateChunkSize = frames;
    }

  double getGeoDistance(double imagePt1[4], double imagePt2[4],
                        double& latDist, double& lonDist);

  void setRealTimeVideoPlayback(bool enable);

  void setRulerEnabled(bool enable);

protected slots:

  void forceUpdate();
  void forceRender();

  void reactToDataChanged();

  void reactToExternalProcessFileChanged(QString);

signals:

  void dataLoaded();
  void dataSetChanged();
  void iconsLoaded();
  void overviewLoaded();
  void followTrackChange(int trackId);

  void trackTypesModified();

  void displayZoom();
  void frameChanged();
  void timeChanged(double microseconds);
  void reachedPlayBoundary();
  void enterAdjudicationMode();
  void criticalError(const QString&);
  void warningError(const QString&);

  void reinitialized();

  void frameRendered();

  void eventFilterChanged();

  void showStatusMessage(const QString& msg, int timeout = 0);

  void filterRegionComplete(vtkPoints* contourPoints, vtkPoints* filterPoints,
                            const vtkVgTimeStamp* timeStamp,
                            vtkMatrix4x4* worldToImageMatrix = 0,
                            const QString& name = QString(),
                            bool enabled = true);
  void temporalFilterReady(int id, const QString& name, int type,
                           double start, double end);

  void mouseMoved(int x, int y);

  void stoppedEditingTrack(vpViewCore::enumAnnotationMode annotationMode);

  void objectInfoUpdateNeeded();

  void trackPicked(int id, int session);

  void trackFiltersChanged();

  void projectOpened();
  void projectClosed();
  void projectVisibilityChanged(int index, bool visible);

  void updateFinished();

  void timeBasedIndexingDisabled();

  void playbackStateChanged();
  void playbackRateChanged(qreal rate);

  void geoDistanceMeasured(double meters);

  void exportFilters(QString path, bool startExternalProcess);

  void removeAllFilters();

  void graphModelExportRequested(QString);

private:

  void updateRepresentation(vtkVgRepresentationBase* representation,
                            vtkObject* filter,
                            unsigned long prevModelUpdateTime,
                            unsigned long modelUpdateTime);

  void enableEventsChange(int eventType, bool state);
  void enableInverseEventsChange(int eventType, bool state);

  void createEventLegend(vpProject* project);
  void updateEventLegendPosition(int w, int h);
  void updateEventLegendColors();

  double getCurrentScale(vtkRenderer* renderer) const;

  void changeIconOffset(int deltaX, int deltaY);
  void changeOverlayOpacity(double delta);
  void changeSceneElementOpacity(double delta);
  void createDisplayOutline(vpProject* project);

  void calculateNewExtents(vtkRenderer* ren, double extents[4]);
  void updateCropExtents(double newExtents[4]);
  void updateAOIImagery();
  void updateViewExtents();

  // Helper functions.
  void handleDataSetNotFound(vpProject* project);
  void handleFileNotFound(const char* tag, const QString& file);

  void initializeImageSource();

  vpProject* loadProject(QScopedPointer<vpProject>& project);

  void addEvents(vpProject* project);
  void removeEvents(vpProject* project);
  void addTrackHeads(vpProject* project);
  void removeTrackHeads(vpProject* project);
  void addTracks(vpProject* project);
  void removeTracks(vpProject* project);

  vtkVgActivity* getHoveredActivity();

  void sanitizeSceneRenderer();

  void initTrackHeadIndicator();
  void initTrackHeadIndicator(vtkPolyData* pd);
  void updateTrackHeadIndicator(int x, int y);
  void updateTrackHeadRegion();

  void updateTrackFollowCamera();

  void worldToImage(double in[2], int out[2]);
  vtkVgGeoCoord worldToGeo(double in[2]);

  bool updateImageMatrices();

  void updateEventDisplayEndFrame(vpProject* project);

  void getMousePosition(int* x, int* y);

  vpProject* findProject(int projectId);

  void focusBounds(double bounds[4], vtkMatrix4x4* xform);

  void startFrameMapRebuild();
  bool waitForFrameMapRebuild();

  void setCurrentFrame(unsigned int frameIndex, double currentTime);
  void setCurrentFrame(const vpFrame& frame, double currentTime);

  void setTrackTrailLength(vpProject* project, vtkVgTimeStamp duration);
  void setSceneElementLineWidth(vpProject* project, double lineWidth);

  vtkVgTrack* makeTrack(int trackId, int session);

  void seekInternal(const vtkVgTimeStamp& position,
                    vg::SeekMode direction = vg::SeekNearest);

  void syncAnimationToCore();

  vpContour* makeFilterContour();

  void nextSingleClickTrack();

  bool UpdatePending;
  bool UpdateObjectViews;
  bool ForceFullUpdate;
  bool RenderPending;
  bool LogRenderTime;
  bool Playing;

  bool DisplayFullVolume;

  bool ShowAnnotations;

  bool ShowTracks;
  bool ShowTrackHeads;
  bool ShowEvents;
  bool ShowEventIcons;
  bool ShowActivities;
  bool ShowSceneElements;

  bool UsingTimeStampData;
  bool UsingTimeBasedIndexing;
  bool UseTimeStampDataIfAvailable;
  bool UseTimeBasedIndexing;

  vpTrackIO::TrackStorageMode TrackStorageMode;
  bool UseGeoCoordinates;
  bool UseRawImageCoordinates;  // ignored if UseGeoCoordinates is true
  bool EnableWorldDisplayIfAvailable;
  bool EnableTranslateImage;
  double ImageTranslationReferenceLatLon[2];
  double ImageTranslationReferenceOffset[2];
  double ImageTranslationOffset[2];

  vtkSmartPointer<vtkMatrix4x4>     YFlipMatrix;
  vtkSmartPointer<vtkMatrix4x4>     ImageToWorldMatrix;
  vtkSmartPointer<vtkMatrix4x4>     WorldToImageMatrix;
  vtkSmartPointer<vtkMatrix4x4>     LatLonToImageMatrix;
  vtkSmartPointer<vtkMatrix4x4>     LatLonToWorldMatrix;
  vtkSmartPointer<vtkMatrix4x4>     LatLonToImageReferenceMatrix;
  vtkSmartPointer<vtkMatrix4x4>     ImageToGcsMatrix;

  int FirstImageY;

  int IconSize;
  int IconOffsetX, IconOffsetY;

  vtkSmartPointer<vtkRenderWindow>  RenderWindow;
  vtkSmartPointer<vtkRenderer>      SceneRenderer;
  vtkSmartPointer<vtkRenderer>      ContextRenderer;
  vtkSmartPointer<vtkRenderer>      BackgroundRenderer;
  vtkSmartPointer<vtkVgInteractorStyleRubberBand2D> RubberbandInteractorStyle;

  vpSessionView*                    SessionView;

  QVTKWidget* RenderWidget;

  // Image
  vtkIdType                             ImageCounter;
  vtkSmartPointer<vtkImageData>         ImageData[2];
  vtkSmartPointer<vtkImageActor>        ImageActor[2];
  vtkSmartPointer<vtkImageData>         AOIImage;
  // keep track of the vtkImageData being displayed in the MainRenderer
  vtkSmartPointer<vtkImageData>         MainImageData;
  vtkSmartPointer<vtkVgBaseImageSource> ImageSource;
  vtkSmartPointer<vtkVgBaseImageSource> ImageSource2;
  double                                ImageSourceLODFactor;
  vtkSmartPointer<vtkActor>             AOIOutlineActor;
  vtkSmartPointer<vtkPolyData>          AOIOutlinePolyData;

  // Scene
  bool SceneInitialized;
  double OverlayOpacity;
  double SceneElementFillOpacity;

  vtkSmartPointer<vtkVgTrackFilter> TrackFilter;
  vtkSmartPointer<vtkVgEventFilter> EventFilter;

  vtkSmartPointer<vtkVgGraphModel> GraphModel;
  vtkSmartPointer<vtkVgGraphRepresentation> GraphRepresentation;

  vtkSmartPointer<vtkLegendBoxActor> EventLegend;

  // Extents / Bounds
  double            ViewExtents[4];
  double            OverviewExtents[4];

  double            WholeImageBounds[6];
  double            OverviewImageBounds[6];

  // Writer
  vtkSmartPointer<vtkWindowToImageFilter> WindowToImageFilter;
  vtkSmartPointer<vtkPNGWriter>           PNGWriter;

  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;

  // General
  bool              AdjudicationMode;
  bool              AdjudicationAllTracksState;
  std::vector<bool> AdjudicationEventsState;

  int               TrackOffset[2];
  int               DisplayMode;

  // Frame
  bool              Loop;
  bool              Paused;
  bool              UseZeroBasedFrameNumbers;
  bool              RightClickToEditEnabled;
  bool              AutoAdvanceDuringCreation;
  double            SceneElementLineWidth;

  int               FrameNumberOffset;

  vtkVgTimeStamp    CoreTimeStamp;
  unsigned int      CurrentFrame;
  unsigned int      LastFrame;
  unsigned int      NumberOfFrames;
  double            CurrentTime;

  vtkSmartPointer<vtkTimerLog> TimeLogger;
  vtkSmartPointer<vtkTimerLog> LoadRenderTimeLogger;

  double            TimeElapsedSinceLastRender;
  double            RenderingTime;
  double            RequestedFPS;

  std::vector<int>  TimerIds;

  // Callbacks
  vtkSmartPointer<vtkVpInteractionCallback>  InteractionCallback;

  // Informatics
  vpInformaticsDialog* InformaticsDialog;

  // Measurements
  vtkSmartPointer<vtkTimerLog> RenderTimeLogger;
  double                       TotalRenderTime;
  double                       TotalNumberOfRenderCalls;

  vpFileDataSource* ImageDataSource;
  vpFrameMap* FrameMap;

  // Configs
  std::vector<vpProject*> Projects;
  vpProjectParser*     ProjectParser;
  int CurrentProjectId;

  vtkVgActivityTypeRegistry* ActivityTypeRegistry;
  vtkVgEventTypeRegistry* EventTypeRegistry;
  vtkVgTrackTypeRegistry* TrackTypeRegistry;

  vpTrackConfig*    TrackConfig;
  vpEventConfig*    EventConfig;
  vpActivityConfig* ActivityConfig;

  vgAttributeSet    TrackAttributes;
  QString           CurrentTrackAttributeGroup;

  int               CurrentTrackColorMode;

  vpNormalcyMaps*   NormalcyMaps;

  vpAnnotation*     Annotation;

  vtkIdType         IdOfTrackToFollow;
  int               FollowTrackProjectId;

  // Render to file
  bool              SaveRenderedImages;
  QString           ImageOutputDirectory;

  vtkSmartPointer<vtkHoverWidget> HoverWidget;
  vtkVgActivity* HoveredActivity;
  bool IsHovering;

  vtkSmartPointer<vtkVgContourOperatorManager> ContourOperatorManager;
  vpContour* Contour;
  std::vector<vpContour*> Contours;
  bool DrawingContour;

  vtkSmartPointer<vtkVgTemporalFilters> TemporalFilters;

  double SavedViewExtents[4];

  int TrackEditProjectId;
  vtkIdType NewTrackId;
  vtkVgTimeStamp NewTrackTimeStamp;
  vtkIdType EditingTrackId;
  double EditedTrackPrevColor[3];
  bool SingleFrameAnnotationMode;

  vpBox* TrackHeadBox;
  vpContour* TrackHeadContour;
  vpBoundingRegion* TrackHeadRegion;
  vtkSmartPointer<vtkPolyData> TrackHeadRegionPolyData;

  vtkSmartPointer<vtkActor> TrackHeadIndicatorActor;
  vtkSmartPointer<vtkPolyData> TrackHeadIndicatorPolyData;

  bool HideTrackHeadIndicator;
  int RegionEditMode;
  int ExternalExecuteMode;
  int ExecutedExternalProcessMode;

  int EventExpirationMode;
  vtkVgTimeStamp ObjectExpirationTime;
  vtkVgTimeStamp TrackTrailLength;

  int TrackUpdateChunkSize;

  vpVideoAnimation* VideoAnimation;

  vtkSmartPointer<vtkPoints> RulerPoints;
  vtkSmartPointer<vtkActor2D> RulerActor;

  std::vector<vtkVgEvent*> SelectedEvents;
  std::vector<vtkVgTrack*> SelectedTracks;

  bool GraphRenderingEnabled;

  bool RulerEnabled;

  QProcess* ExternalProcess;
  QFileSystemWatcher* FileSystemWatcher;
  QString ExternalProcessOutputFile;
  QString ExternalProcessProgram;
  QStringList ExternalProcessArguments;
};

//-----------------------------------------------------------------------------
void vpViewCore::setTrackOffset(int* trackOffset)
{
  if (trackOffset)
    {
    this->TrackOffset[0] = trackOffset[0];
    this->TrackOffset[1] = trackOffset[1];
    }
}

//-----------------------------------------------------------------------------
int vpViewCore::getNumberOfFrames()
{
  return this->NumberOfFrames;
}

//-----------------------------------------------------------------------------
int vpViewCore::getMinimumFrameNumber()
{
  return this->FrameNumberOffset + (this->UseZeroBasedFrameNumbers ? 0 : 1);
}

//-----------------------------------------------------------------------------
int vpViewCore::getMaximumFrameNumber()
{
  return this->getMinimumFrameNumber() + this->NumberOfFrames - 1;
}

//-----------------------------------------------------------------------------
int vpViewCore::getCurrentFrameNumber()
{
  return this->CurrentFrame + this->FrameNumberOffset +
         (this->UseZeroBasedFrameNumbers ? 0 : 1);
}

//-----------------------------------------------------------------------------
unsigned int vpViewCore::getCurrentFrameIndex()
{
  return this->CurrentFrame;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpViewCore::getCoreTimeStamp()
{
  return this->CoreTimeStamp;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpViewCore::getImageryTimeStamp()
{
  return vtkVgTimeStamp(this->CoreTimeStamp.GetTime(), this->CurrentFrame);
}

//-----------------------------------------------------------------------------
double vpViewCore::getRequestFPS() const
{
  return this->RequestedFPS;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpViewCore::getObjectExpirationTime()
{
  return this->ObjectExpirationTime;
}

#endif // __vpViewCore_h
