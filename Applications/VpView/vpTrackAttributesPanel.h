// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
