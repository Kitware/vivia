/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgfNoteTreeModel_h
#define __vgfNoteTreeModel_h

#include "vgfItemModel.h"

struct vgTimeStamp;

class vgfNoteTreeModelPrivate;

/// Note tree model.
///
/// This class provides an item model for a multi-column (i.e. tree) view that
/// displays entity notes in a simplified and (optionally) time-dependent
/// manner.
class VG_GUI_FRAMEWORK_EXPORT vgfNoteTreeModel : public vgfItemModel
{
  Q_OBJECT

  /// This property holds whether inactive items are shown in the item view.
  ///
  /// If this property is \c false, inactive items (that is, items whose
  /// temporal extent does not include the current time) are not shown in the
  /// item view.
  ///
  /// \sa areInactiveItemsShown(), setInactiveItemsShown()
  Q_PROPERTY(bool inactiveItemsShown READ areInactiveItemsShown
                                     WRITE setInactiveItemsShown)

public:
  enum Column
    {
    EntityTypeColumn,
    EntityIconColumn,
    StartTimeColumn,
    EndTimeColumn,
    NoteColumn,
    ColumnCount
    };

public:
  explicit vgfNoteTreeModel(QObject* parent = 0);
  virtual ~vgfNoteTreeModel();

  /// Get whether inactive items are shown in the item view.
  /// \sa inactiveItemsShown, setInactiveItemsShown()
  virtual bool areInactiveItemsShown() const;

  // Reimplemented from QAbstractItemModel
  virtual QVariant data(const QModelIndex& index, int role) const QTE_OVERRIDE;

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const QTE_OVERRIDE;

  // Reimplemented from QSortFilterProxyModel
  virtual bool filterAcceptsRow(
    int sourceRow, const QModelIndex& sourceParent) const QTE_OVERRIDE;

public slots:
  /// Set whether inactive items are shown in the item view.
  /// \sa inactiveItemsShown, areInactiveItemsShown()
  virtual void setInactiveItemsShown(bool);

  /// Set current time.
  ///
  /// This sets the time used to determine which items are "active".
  ///
  /// \sa inactiveItemsShown
  virtual void setCurrentTime(const vgTimeStamp&);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgfNoteTreeModel)

  using vgfItemModel::data;

private:
  QTE_DECLARE_PRIVATE(vgfNoteTreeModel)
  Q_DISABLE_COPY(vgfNoteTreeModel)
};

#endif
