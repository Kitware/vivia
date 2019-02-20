/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgfContextMenu.h"
#include "ui_contextMenu.h"
#include "am_contextMenu.h"

#include <vgCheckArg.h>

#include <QSet>

//-----------------------------------------------------------------------------
class vgfContextMenuPrivate
{
public:
  vgfContextMenuPrivate() : ItemModel(0), JumpFlags(0) {}

  Ui::vgfContextMenu UI;
  Am::vgfContextMenu AM;

  QAbstractItemModel* ItemModel;
  vgf::JumpFlags JumpFlags;

  QList<vgfItemReference> ActiveItems;
};

QTE_IMPLEMENT_D_FUNC(vgfContextMenu)

//-----------------------------------------------------------------------------
vgfContextMenu::vgfContextMenu(QWidget* parent) :
  QMenu(parent), d_ptr(new vgfContextMenuPrivate)
{
  QTE_D(vgfContextMenu);

  d->UI.setupUi(this);
  d->AM.setupActions(d->UI, this);

//   this->addAction(d->UI.actionShowSelected);
//   this->addAction(d->UI.actionHideSelected);
  this->addAction(d->UI.actionJumpToStart);
  this->addAction(d->UI.actionJumpToEnd);

  connect(d->UI.actionJumpToStart, SIGNAL(triggered()),
          this, SLOT(jumpToItemStart()));
  connect(d->UI.actionJumpToEnd, SIGNAL(triggered()),
          this, SLOT(jumpToItemEnd()));
}

//-----------------------------------------------------------------------------
vgfContextMenu::~vgfContextMenu()
{
}

//-----------------------------------------------------------------------------
vgf::JumpFlags vgfContextMenu::jumpFlags() const
{
  QTE_D_CONST(vgfContextMenu);
  return d->JumpFlags;
}

//-----------------------------------------------------------------------------
void vgfContextMenu::setJumpFlags(vgf::JumpFlags flags)
{
  QTE_D(vgfContextMenu);
  d->JumpFlags = flags;
}

//-----------------------------------------------------------------------------
void vgfContextMenu::setModel(QAbstractItemModel* model)
{
  QTE_D(vgfContextMenu);
  d->ItemModel = model;
}

//-----------------------------------------------------------------------------
void vgfContextMenu::setActiveItems(const QModelIndexList& indices)
{
  QTE_D(vgfContextMenu);
  CHECK_ARG(d->ItemModel);

  d->ActiveItems.clear();

  // Get items for selection
  QSet<int> rows;
  foreach (const QModelIndex& index, indices)
    {
    if (!rows.contains(index.row()))
      {
      const QVariant& data =
        d->ItemModel->data(index, vgf::InternalReferenceRole);
      if (data.isValid() && data.canConvert<vgfItemReference>())
        {
        // TODO get other data for other actions
        d->ActiveItems.append(data.value<vgfItemReference>());

        // Don't process the same item multiple times
        rows.insert(index.row());
        }
      }
    }

  // Update enabled state for actions
  const int count = d->ActiveItems.count();
  d->UI.actionJumpToStart->setEnabled(count == 1);
  d->UI.actionJumpToEnd->setEnabled(count == 1);
}

//-----------------------------------------------------------------------------
void vgfContextMenu::activate(const QModelIndex& index, int logicalRole)
{
  QTE_D(vgfContextMenu);
  CHECK_ARG(d->ItemModel);

  // Get item from index
  const QVariant& data =
    d->ItemModel->data(index, vgf::InternalReferenceRole);
  if (data.isValid() && data.canConvert<vgfItemReference>())
    {
    QList<vgfItemReference> activeItems;
    activeItems.append(data.value<vgfItemReference>());

    // Invoke appropriate action, temporarily swapping active items for the
    // item being activated
#if QT_VERSION < 0x040800
    const QList<vgfItemReference> oldItems = d->ActiveItems;
    d->ActiveItems = activeItems;
#else
    activeItems.swap(d->ActiveItems);
#endif
    switch (logicalRole)
      {
      case vgf::EndTimeRole:
        this->jumpToItemEnd();
        break;

      case vgf::StartTimeRole:
      default:
        this->jumpToItemStart();
        break;
      }
#if QT_VERSION < 0x040800
    d->ActiveItems = oldItems;
#else
    activeItems.swap(d->ActiveItems);
#endif
    }
}

//-----------------------------------------------------------------------------
void vgfContextMenu::jumpToItemStart()
{
  QTE_D(vgfContextMenu);
  CHECK_ARG(d->ActiveItems.count() == 1);

  const vgf::JumpFlags flags =
    (d->JumpFlags & ~(vgf::JumpTemporalMask)) | vgf::JumpToStartTime;
  emit this->jumpRequested(d->ActiveItems.first(), flags);
}

//-----------------------------------------------------------------------------
void vgfContextMenu::jumpToItemEnd()
{
  QTE_D(vgfContextMenu);
  CHECK_ARG(d->ActiveItems.count() == 1);

  const vgf::JumpFlags flags =
    (d->JumpFlags & ~(vgf::JumpTemporalMask)) | vgf::JumpToEndTime;
  emit this->jumpRequested(d->ActiveItems.first(), flags);
}
