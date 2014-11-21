/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackTreeModel_h
#define __vsTrackTreeModel_h

#include <QAbstractItemModel>

#include <vtkSmartPointer.h>

#include <vvTrack.h>

class QColor;
class QPixmap;

class vgSwatchCache;

class vtkVgTrack;
class vtkVgTrackFilter;

class vsCore;
class vsScene;

class vsTrackTreeColorHelper
{
public:
  virtual ~vsTrackTreeColorHelper() {}
  virtual QColor color(vtkVgTrack*) = 0;
};

class vsTrackTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum ModelColumn
    {
    NameColumn,
    TrackTypeColumn,
    ProbabilityColumn,
    StartTimeColumn,
    EndTimeColumn,
    IdColumn // not displayed
    };

  enum { NumColumns = IdColumn + 1 };

  enum DataRole
    {
    LogicalIdRole = Qt::UserRole + 1,
    DisplayStateRole
    };

public:
  vsTrackTreeModel(vsCore* core, vsScene* scene, vtkVgTrackFilter* trackFilter,
                   QObject* parent = 0);
  virtual ~vsTrackTreeModel();

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

  bool isIndexHidden(const QModelIndex& index) const;

  QModelIndex indexOfTrack(vtkIdType trackId) const;

  void setColor(int type, const QColor& color);
  void setColorHelper(vsTrackTreeColorHelper*);

public slots:
  void addTrack(vtkVgTrack* track);
  void updateTrack(vtkVgTrack* track);
  void removeTrack(vtkIdType trackId);
  void update();

protected:
  void setTrackDisplayState(const QModelIndex& index, bool show);

  QPixmap colorSwatch(const QColor&) const;

protected slots:
  void deferredAddTracks();
  void deferredUpdateTracks();

private:
  Q_DISABLE_COPY(vsTrackTreeModel)

  vsCore* Core;
  vsScene* Scene;
  vtkSmartPointer<vtkVgTrackFilter> trackFilter;

  const vgSwatchCache& swatchCache;

  vsTrackTreeColorHelper* colorHelper;

  QList<vtkVgTrack*> tracks;
  QMap<vtkIdType, vtkVgTrack*> addedTracks;
  QList<vtkVgTrack*> updatedTracks;
  QMap<int, QColor> colors;

  bool isSorted;
};

Q_DECLARE_METATYPE(vvTrackId)

#endif
