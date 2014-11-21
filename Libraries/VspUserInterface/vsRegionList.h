/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsRegionList_h
#define __vsRegionList_h

#include <QTreeWidget>

#include <vsContour.h>
#include <vsEventInfo.h>

class vsRegionListPrivate;

class vsRegionList : public QTreeWidget
{
  Q_OBJECT

public:
  vsRegionList(QWidget* parent);
  ~vsRegionList();

  void addContextMenuAction(QAction*);

signals:
  void regionNameChanged(int id, QString newName);
  void regionTypeChanged(int id, vsContour::Type newType);
  void regionVisibilityChanged(int id, bool visible);
  void regionRemoved(int id);
  void regionConvertedToEvent(int id, int eventType);

  void selectionStatusChanged(bool selectionIsNonEmpty);

public slots:
  void addRegion(vsContour);
  void setRegionName(int id, QString);
  void setRegionType(int id, vsContour::Type);
  void removeRegion(int id);

  void removeSelected();
  void removeAll();

  void setShowHidden(bool);

  void setEventTypes(QList<vsEventInfo>);

  void convertSelectedToEvents(int type);

protected slots:
  void updateRegionFromItem(QTreeWidgetItem* item, int column);
  void updateSelectionStatus();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsRegionList)

  virtual void contextMenuEvent(QContextMenuEvent*) QTE_OVERRIDE;

  void emitRemovalStatus(int numRemoved);

private:
  QTE_DECLARE_PRIVATE(vsRegionList)
  Q_DISABLE_COPY(vsRegionList)
};

#endif
