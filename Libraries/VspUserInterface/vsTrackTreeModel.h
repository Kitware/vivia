/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackTreeModel_h
#define __vsTrackTreeModel_h

#include <vsTrackId.h>

#include <qtGlobal.h>

#include <vtkSmartPointer.h>

#include <QAbstractItemModel>

class QColor;
class QPixmap;

class vgSwatchCache;

class vtkVgTrack;
class vtkVgTrackFilter;

class vsCore;
class vsScene;

class vsTrackTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum ModelColumn
    {
    NameColumn,
    StarColumn,
    TrackTypeColumn,
    ProbabilityColumn,
    StartTimeColumn,
    EndTimeColumn,
    NoteColumn,
    IdColumn // not displayed
    };

  enum { NumColumns = IdColumn + 1 };

  enum DataRole
    {
    LogicalIdRole = Qt::UserRole + 1,
    DisplayStateRole,
    StarRole,
    };

public:
  vsTrackTreeModel(vsCore* core, vsScene* scene, vtkVgTrackFilter* trackFilter,
                   QObject* parent = 0);
  virtual ~vsTrackTreeModel();

  // Reimplemented from QAbstractItemModel
  virtual Qt::ItemFlags flags(const QModelIndex& index) const QTE_OVERRIDE;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const QTE_OVERRIDE;

  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const QTE_OVERRIDE;

  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role);

  virtual QModelIndex index(int row, int column,
                            const QModelIndex& parent = QModelIndex()) const QTE_OVERRIDE;

  virtual QModelIndex parent(const QModelIndex& index) const QTE_OVERRIDE;

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const QTE_OVERRIDE;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const QTE_OVERRIDE;

  bool isIndexHidden(const QModelIndex& index) const;

  QModelIndex indexOfTrack(vtkIdType trackId) const;

  QList<vtkVgTrack*> trackList() const
    {
    return tracks;
    }

signals:
  void trackNoteChanged(vtkIdType trackId, QString note);

public slots:
  void addTrack(vtkVgTrack* track);
  void updateTrack(vtkVgTrack* track);
  void removeTrack(vtkIdType trackId);
  void update();

protected:
  void setTrackDisplayState(const QModelIndex& index, bool show);

  QPixmap colorSwatch(const QColor&) const;

protected slots:
  void deferredUpdateTracks();

private:
  QTE_DISABLE_COPY(vsTrackTreeModel)

  vsCore* Core;
  vsScene* Scene;
  vtkSmartPointer<vtkVgTrackFilter> trackFilter;

  const vgSwatchCache& swatchCache;

  QList<vtkVgTrack*> tracks;
  QMap<vtkIdType, vtkVgTrack*> addedTracks;
  QList<vtkVgTrack*> updatedTracks;

  bool isSorted;
};

Q_DECLARE_METATYPE(vsTrackId)

#endif
