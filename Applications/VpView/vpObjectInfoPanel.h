/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpObjectInfoPanel_h
#define __vpObjectInfoPanel_h

#include <QWidget>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include "vtkVgTimeStamp.h"

namespace Ui
{
class vpObjectInfoPanel;
}

class vpTrackConfig;
class vpTrackIO;
class vpViewCore;
class vtkVpTrackModel;
class vtkVgActivityManager;
class vtkVgEvent;
class vtkVgEventModel;
class vtkVgEventTypeRegistry;
class vtkVgTrack;

class vpObjectInfoPanel : public QWidget
{
  Q_OBJECT

public:
  vpObjectInfoPanel(QWidget* parent = 0);
  virtual ~vpObjectInfoPanel();

  void ShowActivityInfo(int index);
  void ShowEventInfo(int id, int parentId, int index);
  void ShowTrackInfo(int id, int parentId, int index);
  void ShowSceneElementInfo(int id);
  void ShowEmptyPage();

  void Initialize(vpViewCore* viewCore, vtkVgActivityManager* activityManager,
                  vtkVgEventModel* eventModel, vtkVpTrackModel* trackModel,
                  vtkVgEventTypeRegistry* eventTypes, vpTrackConfig* trackTypes,
                  const vpTrackIO* trackIO);

signals:
  void ObjectIdChanged(int objectType, int newId);
  void ObjectTypeChanged(int objectType, int id);
  void TypeColorChanged(int typeIndex, double *rgb);

private slots:
  void StartFrameChanged(int val);
  void EndFrameChanged(int val);

  void EditProperties();

  void UpdateTrackType(int type);

private:
  void EditTrackInfo();
  void EditParentTrackInfo();

  void ShowTrackInfo(int id);
  void ShowParentTrackInfo(int id, int parentId, int index);

  bool FindClosestValidFrame(vtkVgTimeStamp& current, bool next);

  void UpdateParentTrackEventTimes();

  void SetEditProperties(bool edit);

  void Cleanup();

  void BuildTypeList();
  void SetCurrentType(int index);
  int  GetCurrentType();

private:
  Ui::vpObjectInfoPanel* Ui;

  vpViewCore*            ViewCoreInstance;
  vtkVgActivityManager*  ActivityManager;
  vtkVgEventModel*       EventModel;
  vtkVpTrackModel*       TrackModel;
  const vpTrackIO*       TrackIO;

  vtkVgEventTypeRegistry* EventTypeRegistry;
  vpTrackConfig* TrackConfig;

  int TrackIndex;
  vtkSmartPointer<vtkVgTrack> Track;
  vtkSmartPointer<vtkVgEvent> ChildEvent;

  int LastStartFrameVal, LastEndFrameVal;
  bool Editing;

  vtkVgTimeStamp InitialStartFrame;
  vtkVgTimeStamp InitialEndFrame;
  bool ParentTrackChangesToApply;

  int PrevType;
  int TrackTypeRGB[3];
};

#endif
