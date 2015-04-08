/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgfContextMenu_h
#define __vgfContextMenu_h

#include "vgfItemReference.h"
#include "vgfNamespace.h"

#include <qtGlobal.h>

#include <vgExport.h>

#include <QMenu>
#include <QModelIndex>

class QAbstractItemModel;

class vgfContextMenuPrivate;

/// Generic item context menu.
///
/// This class provides a common context menu and shortcut actions for item
/// views. When associated with an item model and connected to the selection
/// signals for the same, it provides automatic enabling and disabling of
/// actions, as appropriate. Some actions (e.g. note editing) also provide
/// function implementations as well. The menu emits signals to notify the
/// application when action should be taken to update item information (e.g.
/// item note, visibility state, etc.) or to perform other actions requested by
/// the user (e.g. jump to item).
///
/// Users may add the common actions to their own context (i.e. by calling
/// actions() on the context menu instance and QWidget::addAction() to copy the
/// actions into their own context) in order to allow the user to activate the
/// actions by their shortcut key sequences without the need to open the menu.
/// The provided common actions use Qt::WidgetWithChildrenShortcut as their
/// shortcut context, so that different views may each use their own instance
/// of the actions to operate on their own selections without conflicting with
/// other views that may have different item selections.
///
/// Because it is a subclass of QMenu, users are free to add their own actions
/// in addition to the common set that is provided.
class VG_GUI_FRAMEWORK_EXPORT vgfContextMenu : public QMenu
{
  Q_OBJECT

  /// Flags controlling the default behavior of 'jump to' actions.
  ///
  /// This specifies the default flags that are used when emitting
  /// jumpToItem(). The specific action selected by the user may alter these
  /// flags. Typically, this is set to the desired spatial jump flags, while
  /// the user's action choice determines the temporal jump flags.
  ///
  /// \sa jumpFlags(), setJumpFlags()
  Q_PROPERTY(vgf::JumpFlags jumpFlags READ jumpFlags WRITE setJumpFlags)

public:
  explicit vgfContextMenu(QWidget* parent = 0);
  virtual ~vgfContextMenu();

  /// Get jump flags for 'jump to' actions.
  /// \sa jumpFlags, setJumpFlags()
  vgf::JumpFlags jumpFlags() const;

  /// Set the item model for the context menu.
  ///
  /// This sets the item model that the context menu will use to retrieve
  /// information about active items when using setActiveItems().
  void setModel(QAbstractItemModel*);

public slots:
  /// Set jump flags for 'jump to' actions.
  /// \sa jumpFlags
  void setJumpFlags(vgf::JumpFlags);

  /// Set active items from model indices.
  ///
  /// This sets the menu 'active items' &mdash; that is, the items to which
  /// the menu actions will apply &mdash; to the items identified by the
  /// specified item model indices. Setting the active items automatically
  /// enables and/or disables built-in actions based on their applicability to
  /// the active item set.
  void setActiveItems(const QModelIndexList&);

  /// Invoke appropriate action from item activation.
  ///
  /// This method invokes an appropriate default action on an item view index
  /// that has been activated (a la QAbstractItemView::activate()), using the
  /// logical data role to determine the appropriate action.
  ///
  /// For example, calling activate() with the role vgf::EndTimeRole might emit
  /// a jump request for the item's end time.
  void activate(const QModelIndex&, int logicalRole);

signals:
  /// Emitted when the user requests to jump to an item.
  void jumpRequested(vgfItemReference, vgf::JumpFlags);

protected slots:
  void jumpToItemStart();
  void jumpToItemEnd();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgfContextMenu)

private:
  QTE_DECLARE_PRIVATE(vgfContextMenu)
  QTE_DISABLE_COPY(vgfContextMenu)
};

#endif
