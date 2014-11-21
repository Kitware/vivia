/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsEventTreeModel_h
#define __vsEventTreeModel_h

#include <QAbstractItemModel>
#include <QIcon>

#include <vtkSmartPointer.h>

#include "vsEventUserInfo.h"

class QPixmap;

class vgSwatchCache;

class vtkVgEvent;
class vtkVgEventFilter;
class vtkVgEventTypeRegistry;

class vsScene;

class vsEventTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum ModelColumn
    {
    RatingColumn,
    IdColumn, // not displayed
    NameColumn,
    StarColumn,
    EventTypeColumn,
    ProbabilityColumn,
    StartTimeColumn,
    EndTimeColumn,
    NoteColumn
    };

  enum { NumColumns = NoteColumn + 1 };

  enum DataRole
    {
    DisplayStateRole = Qt::UserRole + 1,
    RatingRole,
    StatusRole,
    StarRole,
    NameSortRole,
    ModifiableRole
    };

public:
  vsEventTreeModel(vsScene* scene, vtkVgEventFilter* eventFilter,
                   vtkVgEventTypeRegistry* eventTypeRegistry,
                   const vgSwatchCache& swatchCache, QObject* parent = 0);
  virtual ~vsEventTreeModel();

  // Reimplemented from QAbstractItemModel
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  virtual QVariant data(const QModelIndex& index, int role) const;

  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const;

  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role);

  virtual QModelIndex index(int row, int column,
                            const QModelIndex& parent = QModelIndex()) const;

  virtual QModelIndex parent(const QModelIndex& index) const;

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  virtual QStringList mimeTypes() const;
  virtual QMimeData* mimeData(const QModelIndexList& indexes) const;

  bool isIndexHidden(const QModelIndex& index) const;

  QModelIndex indexOfEvent(vtkIdType eventId) const;

  QList<vsEventUserInfo> eventList() const
    {
    return events;
    }

signals:
  void eventRatingChanged(vtkIdType eventId, int rating);
  void eventStatusChanged(vtkIdType eventId, int status);
  void eventNoteChanged(vtkIdType eventId, QString note);

public slots:
  void addEvent(vtkVgEvent* event);
  void updateEvent(vtkVgEvent* event);
  void removeEvent(vtkIdType eventId);
  void update();

protected:
  void setEventDisplayState(const QModelIndex& index, bool show);

  QPixmap colorSwatch(const QColor&) const;

protected slots:
  void deferredAddEvents();
  void deferredUpdateEvents();

private:
  Q_DISABLE_COPY(vsEventTreeModel)

  vsScene* Scene;

  vtkSmartPointer<vtkVgEventFilter> eventFilter;
  vtkSmartPointer<vtkVgEventTypeRegistry> eventTypeRegistry;

  const vgSwatchCache& swatchCache;

  QList<vsEventUserInfo> events;
  QMap<vtkIdType, vtkVgEvent*> addedEvents;
  QList<vtkVgEvent*> updatedEvents;
  QIcon yesIcon;
  QIcon noIcon;

  bool isSorted;
};

#endif
