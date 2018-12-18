/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqApplication_h
#define __vqApplication_h

#include "ui_viqui.h"

#include <vvQueryInstance.h>

#include <qtUiState.h>

#include <QMainWindow>
#include <QScopedPointer>

class QSignalMapper;

class pqCoreTestUtility;

class qtCliArgs;
class qtDockController;

class vqCore;
class vqUserActions;
class vqQueryDialog;
class vqTrackingClipViewer;

struct vvSimilarityQuery;

class vqApplication : public QMainWindow
{
  Q_OBJECT

public:
  enum UIMode
    {
    UI_Engineering,
    UI_Analyst
    };

public:
  vqApplication(UIMode uiMode);
  virtual ~vqApplication();

  bool useAdvancedUI()
    {
    return this->InterfaceMode == UI_Engineering;
    }

public slots:
  void initializeTesting(const qtCliArgs*);
  void addLayer(QUrl);

  void resultsPageBack();
  void resultsPageForward();

protected slots:
  void showQueryNewDialog();
  void showQueryEditDialog();
  void showQueryOpenDialog();
  void showGroundTruthOpenDialog();
  void showLayerAddFileDialog();
  void showConfigureDialog();
  void refineResults();

  void setResultFilters();

  void showQueryClip(vvQueryInstance query);

  void showTrackingClips(QList<vtkVgVideoNode*> nodes);
  void showNextTrackingClips(vtkIdType previousId, int count);

  void setUserScore(long long iid, int score);
  void setResultNote(long long iid, const QString& note);
  void setResultStarred(long long iid, bool starred);

  void setClipRating(int id, int rating);

  void showBestClips();

  void updateTrackingClipViewer(bool clear);
  void selectedClip(int id);
  void activatedClip(int id);
  void trackingClipViewerClosed();

  void refreshResults();
  void acceptResultSet(bool haveSession, bool scoreRequestedResults);

  void executeNewQuery();
  void cancelNewQuery();
  void formulateQuery(vvProcessingRequest request, long long initialTime);

  void exportSelectedResults(const QString& exporterId);
  void exportStarredResults(const QString& exporterId);
  void exportAllResults(const QString& exporterId);

  void setGroundTruthEventType(int index);

  void showResultDetails(QList<vtkVgNodeBase*> nodes);

  void updateVideoPlayerTitle(vtkVgVideoNode* node);

  void findResult();

  void reloadPredefinedQueryCache();

  void generateReport();

  void exportKml();

protected:
  virtual void closeEvent(QCloseEvent*);

  void reloadConfiguration();
  void connectDockToggleAction(QAction* action, QDockWidget* dock);
  void connectTreeWidget(vqTreeView*);
  void setEnabledSignal(QAction* action, QObject* sender, const char* signal,
                        bool initiallyEnabled = false);

  void updateResultPageControls();

  void setupTrackingClipViewer();

  UIMode InterfaceMode;

  Ui::MainWindow UI;
  qtUiState UiState;

  vqCore* Core;
  vqUserActions* UserActions;

  int ResultPageOffset;

  QScopedPointer<vqQueryDialog> NewQueryDialog;
  QScopedPointer<vqTrackingClipViewer> TrackingClipViewer;

  qtDockController* DockController;

  pqCoreTestUtility* TestUtility;
};

#endif
