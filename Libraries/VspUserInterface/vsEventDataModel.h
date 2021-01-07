// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventDataModel_h
#define __vsEventDataModel_h

#include <vsEvent.h>

#include <qtGlobal.h>

#include <vtkType.h>

#include <QAbstractItemModel>

class vtkVgEvent;

class vsScene;

class vsEventDataModelPrivate;

class vsEventDataModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit vsEventDataModel(vsScene*, QObject* parent = 0);
  virtual ~vsEventDataModel();

  // Reimplemented from QAbstractItemModel
  virtual QVariant data(const QModelIndex& index, int role) const QTE_OVERRIDE;
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

public slots:
  void addEvent(vtkVgEvent* event, const vsEventId& eventId);
  void updateEvent(vtkVgEvent* event);
  void removeEvent(vtkIdType eventId);

  void invalidate();

protected slots:
  void processUpdates();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsEventDataModel)

private:
  QTE_DECLARE_PRIVATE(vsEventDataModel)
  Q_DISABLE_COPY(vsEventDataModel)
};

#endif
