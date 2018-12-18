/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpTrackAttributesPanel_h
#define __vpTrackAttributesPanel_h

#include <QWidget>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include "vtkVgTimeStamp.h"

namespace Ui
{
class vpTrackAttributesPanel;
}

class QTreeWidgetItem;

class vgAttributeSet;
class vpProject;
class vpViewCore;
class vtkVgTrack;
class vtkVgTrackModel;

class vpTrackAttributesPanel : public QWidget
{
  Q_OBJECT

public:
  vpTrackAttributesPanel(QWidget* parent = 0);
  virtual ~vpTrackAttributesPanel();

  void ShowTrackAttributesControls(bool state);
  void UpdateTrackAttributes(int id);
  double GetTrackAttributesValue();

  void SetEditing(bool state)
    {
    this->Editing = state;
    }

  void Initialize(vpViewCore* viewCore, vpProject* project);

private slots:
  void ToggleDisplayOnlyActiveAttributes(int state);
  void UpdateTrackAttribute(QTreeWidgetItem*);

private:
  void updateEditable();
  void BuildAttributeTree();

private:
  Ui::vpTrackAttributesPanel* Ui;

  vtkSmartPointer<vtkVgTrack> Track;

  vpViewCore*            ViewCoreInstance;
  vpProject*             Project;

  bool Editing;
};

#endif
