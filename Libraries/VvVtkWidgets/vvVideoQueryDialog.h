// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvVideoQueryDialog_h
#define __vvVideoQueryDialog_h

#include <qtGlobal.h>

#include <vtkType.h>

#include <vgExport.h>

#include <vvDescriptor.h>

#include "vvAbstractSimilarityQueryDialog.h"

class QTreeWidget;
class QTreeWidgetItem;

class qtStatusManager;

class vvDescriptorInfoTree;

class vtkVgTimeStamp;

class vvQueryVideoPlayer;
class vvVideoQueryDialogPrivate;

class VV_VTKWIDGETS_EXPORT vvVideoQueryDialog
  : public vvAbstractSimilarityQueryDialog
{
  Q_OBJECT

public:
  ~vvVideoQueryDialog();

  void initialize(vgTimeStamp initialTime = vgTimeStamp());

  virtual std::vector<vvTrack> selectedTracks() const;
  virtual std::vector<vvDescriptor> selectedDescriptors() const;

  virtual void setSelectedDescriptors(const std::vector<vvDescriptor>&);

public slots:
  virtual int exec();

protected slots:
  void setSelectionControlsEnabled(bool state);

  virtual void chooseQueryVideo() {}
  virtual void reprocessQueryVideo() {}

  virtual void setQueryTracksAndDescriptors(
    QList<vvDescriptor> descriptors, QList<vvTrack> tracks);
  virtual void clearQueryDescriptors();

  void moveToSelected();
  void moveToAvailable();
  void clearSelected();

  void setStartTimeConstraintToCurrentFrame();
  void setEndTimeConstraintToCurrentFrame();
  void clearTimeConstraints();

  void addKeyframe();
  void removeSelectedKeyframes();
  void loadKeyframes();
  void unsetKeyframeEditCursor();
  void finalizeKeyframeEdit();
  void cancelKeyframeEdit();

  void selectDescriptor(vtkIdType id);
  void selectKeyframe(vtkIdType id);
  void selectTrack(vtkIdType id);
  void selectItem(vvDescriptorInfoTree* tree, vtkIdType id, int type);

  void selectItem(QTreeWidgetItem* item, int column);

  void showAll();
  void hideAll();
  void hideAllDescriptors();
  void itemVisibilityChanged(QTreeWidgetItem* item, int column);
  void updateItemParentVisibilities();

  void updateSelectionControlsState();
  void updateTrackConstraintControlsState();
  void updateKeyframeControlsState();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvVideoQueryDialog)

  vvVideoQueryDialog(vvVideoQueryDialogPrivate*, bool useAdvancedUi,
                     QWidget* parent = 0, Qt::WindowFlags flags = 0);

  void moveToSelected(QList<qint64> ids);

  void fillTrackDescriptors();
  void fillRegionDescriptors();

  void updateTrackMatchingDescriptors();
  void updateKeyframeMatchingDescriptors();

private:
  QTE_DECLARE_PRIVATE(vvVideoQueryDialog)
};

#endif
