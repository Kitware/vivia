/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvDescriptorInfoTree_h
#define __vvDescriptorInfoTree_h

#include <QTreeWidget>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvDescriptor.h>

#include "vvDescriptorStyle.h"

class vvDescriptorInfoTree;
class vvDescriptorInfoTreePrivate;

//-----------------------------------------------------------------------------
class VV_WIDGETS_EXPORT vvDescriptorInfoTreeItemFactory
{
public:
  virtual ~vvDescriptorInfoTreeItemFactory();

protected:
  QTE_DECLARE_PUBLIC_PTR(vvDescriptorInfoTree)
  friend class vvDescriptorInfoTreePrivate;

  vvDescriptorInfoTreeItemFactory(vvDescriptorInfoTree* tree);

  virtual QTreeWidgetItem* createItem(qint64 id, const vvDescriptor&);
  virtual QTreeWidgetItem* createGroupItem(int style);

private:
  QTE_DECLARE_PUBLIC(vvDescriptorInfoTree)
};

//-----------------------------------------------------------------------------
class VV_WIDGETS_EXPORT vvDescriptorInfoTree : public QTreeWidget
{
  Q_OBJECT

  Q_PROPERTY(bool groupByStyle READ groupByStyle WRITE setGroupByStyle)

public:
  enum DataRole
    {
    IdRole = Qt::UserRole,
    TypeRole,
    ChildDescriptorsRole,
    StyleRole,
    UserRole = Qt::UserRole + 32
    };

  enum ItemType
    {
    Unknown = 0,
    DescriptorItem,
    DescriptorStyleGroup,
    UserType = 32
    };

  enum Column
    {
    Name        = 0x1,
    Source      = 0x2,
    TimeRange   = 0x4,
    All         = 0xf
    };
  Q_DECLARE_FLAGS(Columns, Column)

  explicit vvDescriptorInfoTree(QWidget* parent = 0);
  virtual ~vvDescriptorInfoTree();

  void setItemFactory(vvDescriptorInfoTreeItemFactory*);

  QHash<qint64, vvDescriptor> descriptors() const;
  QList<vvDescriptor> descriptors(QList<qint64>) const;
  QList<vvDescriptor> descriptors(QList<QTreeWidgetItem*>,
                                  bool includeChildren = false) const;
  QList<qint64> descriptorIds(QList<QTreeWidgetItem*>,
                              bool includeChildren = false) const;

  QTreeWidgetItem* findItem(int type, qint64 id, QTreeWidgetItem* from = 0);
  QList<QTreeWidgetItem*> findItems(int type, qint64 id = -1);

  QList<QTreeWidgetItem*> descriptorItems(qint64 id = -1);

  bool groupByStyle() const;
  void setStyleMap(vvDescriptorStyle::Map);
  void setColumns(Columns columns, bool setHeaders = true);

  static qint64 itemId(const QTreeWidgetItem*, bool* isValid = 0);
  static int itemType(const QTreeWidgetItem*);
  static void setItemId(QTreeWidgetItem*, qint64);
  static void setItemType(QTreeWidgetItem*, int);

signals:
  void descriptorSelectionChanged(QList<qint64>);
  void descriptorSelectionChanged(QList<vvDescriptor>);

public slots:
  void setDescriptors(QHash<qint64, vvDescriptor>);
  void addDescriptorItems(QList<qint64>, QTreeWidgetItem* parentItem = 0);
  void setDescriptorItems(QList<qint64>, QTreeWidgetItem* parentItem = 0);
  void clear();

  void setGroupByStyle(bool);
  void setHideGroupedItems(bool);

protected slots:
  void emitSelectionChanges();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvDescriptorInfoTree)

private:
  QTE_DECLARE_PRIVATE(vvDescriptorInfoTree)
  Q_DISABLE_COPY(vvDescriptorInfoTree)

  friend class vvDescriptorInfoTreeItemFactory;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vvDescriptorInfoTree::Columns)

#endif
