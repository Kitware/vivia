// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventTreeModel_h
#define __vsEventTreeModel_h

#include <QAbstractItemModel>
#include <QIcon>

#include <qtGlobal.h>

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
  virtual Qt::ItemFlags flags(const QModelIndex& index) const QTE_OVERRIDE;

  virtual QVariant data(
    const QModelIndex& index, int role = Qt::DisplayRole) const QTE_OVERRIDE;

  virtual QVariant headerData(
    int section, Qt::Orientation orientation,
    int role = Qt::DisplayRole) const QTE_OVERRIDE;

  virtual bool setData(
    const QModelIndex& index, const QVariant& value, int role) QTE_OVERRIDE;

  virtual QModelIndex index(
    int row, int column,
    const QModelIndex& parent = QModelIndex()) const QTE_OVERRIDE;

  virtual QModelIndex parent(const QModelIndex& index) const QTE_OVERRIDE;

  virtual int rowCount(
    const QModelIndex& parent = QModelIndex()) const QTE_OVERRIDE;
  virtual int columnCount(
    const QModelIndex& parent = QModelIndex()) const QTE_OVERRIDE;

  virtual QStringList mimeTypes() const QTE_OVERRIDE;
  virtual QMimeData* mimeData(
    const QModelIndexList& indexes) const QTE_OVERRIDE;

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
  void deferredUpdateEvents();

private:
  QTE_DISABLE_COPY(vsEventTreeModel)

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
