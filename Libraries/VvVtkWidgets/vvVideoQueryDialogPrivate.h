/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvVideoQueryDialog.h"
#include "ui_videoQuery.h"

#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QSettings>

// Qt Extensions includes
#include <qtScopedValueChange.h>
#include <qtStatusManager.h>
#include <qtUtil.h>

#include <vgExport.h>

// visgui includes
#include <vgFileDialog.h>

// VTK Extensions includes
#include <vtkVgEvent.h>
#include <vtkVgEventRegionRepresentation.h>
#include <vtkVgRegionWidget.h>

#include <vvDescriptorInfoTree.h>

#include "vvQueryVideoPlayer.h"

namespace // anonymous
{

enum DisplayFlag
{
  UnselectedFlag = vvQueryVideoPlayer::EventFlag,
  SelectedFlag = vvQueryVideoPlayer::UserFlag
};

enum ItemType
{
  DescriptorItem = vvDescriptorInfoTree::DescriptorItem,
  DescriptorStyleGroup = vvDescriptorInfoTree::DescriptorStyleGroup,
  TrackItem = vvDescriptorInfoTree::UserType,
  KeyframeItem
};

enum DataRole
{
  StartTimeConstraintRole = vvDescriptorInfoTree::UserRole,
  EndTimeConstraintRole,
  VisibilityRole
};

enum TreeColumns
{
  // Common
  NameColumn = 0,
  // Descriptors by Track
  SourceColumn = 1,
  StartTimeColumn = 2,
  EndTimeColumn = 3,
  // Descriptors by Region
  RegionColumn = 1,
  TimeColumn = 2
};

enum
{
  TracklessId = -2  // avoiding -1 because it is reserved for don't match by id
};

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class VV_VTKWIDGETS_EXPORT vvVideoQueryDialogPrivate
{
public:
  vvVideoQueryDialogPrivate(vvVideoQueryDialog* q,
                            vvQueryVideoPlayer* player);
  ~vvVideoQueryDialogPrivate();

  vvDescriptorInfoTree* currentAvailableTree() const;
  QList<qint64> selectedDescriptorIds(vvDescriptorInfoTree* tree = 0) const;

  QTreeWidgetItem* createKeyframeItem(vtkIdType id) const;

  void setEventSelectionState(vtkIdType id, bool state);
  void setDescriptorEnabled(qint64 id, bool state);

  void setEntityVisibility(bool visibility, int excludeType = -1);

  void scheduleParentVisibilityUpdate(QTreeWidgetItem* item,
                                      Qt::CheckState hint);

  bool setTimeConstraint(int role);
  bool setTimeConstraint(int role, vgTimeStamp value);

  void createRegionWidget();
  void editKeyframe(vtkIdType id);

  static void setItemEnabled(QTreeWidgetItem*, bool state,
                             QTreeWidget* tree = 0);

  static vgTimeStamp timeConstraint(QTreeWidgetItem*, int role,
                                    vgTimeStamp defaultValue = vgTimeStamp());
  static void setTimeConstraint(QTreeWidgetItem*, int role,
                                vgTimeStamp value);

  static void updateKeyframeItem(QTreeWidgetItem* item, vtkIdType id,
                                 const vgRegionKeyframe& keyframe);

  static bool isMetadataDescriptor(const vvDescriptor&);

  Ui::VideoQueryDialog UI;

  vvQueryVideoPlayer* Player;
  vgTimeStamp InitialTime;

  QLabel* StatusLabel;
  QProgressBar* StatusProgress;
  qtStatusSource StatusSource;
  qtStatusManager StatusManager;

  QScopedPointer<vtkVgRegionWidget> RegionWidget;
  vtkIdType NewKeyframeId;
  vtkIdType EditKeyframeId;
  vgTimeStamp EditKeyframeTime;

  QList<vvTrack> Tracks;

  vtkSmartPointer<vtkVgEventRegionRepresentation> SelectedEventRepresentation;
  QHash<qint64, vvDescriptor> Descriptors;
  QList<vvDescriptor> MetadataDescriptors;

  QHash<qint64, QSet<qint64> > DescriptorsByTrackId;
  QHash<vtkIdType, vgRegionKeyframe> Keyframes;
  QSet<qint64> SelectedDescriptors;

  std::vector<vvDescriptor> DeferredSelectedDescriptors;

  QHash<QTreeWidgetItem*, Qt::CheckState> PendingVisibilityUpdates;


  bool UseAdvancedUi;

protected:
  QTE_DECLARE_PUBLIC_PTR(vvVideoQueryDialog)

  void setEntityVisibility(QTreeWidget* tree, bool visibility, int excludeType,
                           QSet<vtkIdType>& updatedEvents);

private:
  QTE_DECLARE_PUBLIC(vvVideoQueryDialog)
};
