// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsAlertList_h
#define __vsAlertList_h

#include <QTreeWidget>

#include <qtGlobal.h>

#include "vsAlert.h"

class vgSwatchCache;

class vsAlertEditor;

class vsAlertListPrivate;

class vsAlertList : public QTreeWidget
{
  Q_OBJECT

public:
  vsAlertList(QWidget* parent);
  ~vsAlertList();

  void setSwatchCache(const vgSwatchCache&);

signals:
  void alertChanged(int id, vsAlert);
  void alertActivationChanged(int id, bool active);
  void alertRemoved(int id);

public slots:
  void addAlert(int id, vsAlert);
  void updateAlert(int id, vsAlert);
  void updateAlertThreshold(int id, double);
  void setAlertMatches(int id, int matches);
  void setAlertEnabled(int id, bool enabled);
  void removeAlert(int id);

  void removeSelected();
  void removeAll();

  void activateSelected();
  void deactivateSelected();
  void setSelectedActivation(bool state);
  void setShowInactive(bool);

protected slots:
  void editAlertChanged();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsAlertList)

  virtual bool edit(const QModelIndex& index, QAbstractItemView::EditTrigger,
                    QEvent*);

private:
  QTE_DECLARE_PRIVATE(vsAlertList)
  Q_DISABLE_COPY(vsAlertList)
};

#endif
