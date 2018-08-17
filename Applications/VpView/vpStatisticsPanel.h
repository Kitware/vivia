/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpStatisticsPanel_h
#define __vpStatisticsPanel_h

#include <qtGlobal.h>

#include <QWidget>

class vpViewCore;

class vpStatisticsPanelPrivate;

class vpStatisticsPanel : public QWidget
{
  Q_OBJECT

public:
  vpStatisticsPanel(QWidget* parent = nullptr);
  virtual ~vpStatisticsPanel();

  void bind(vpViewCore*);

public slots:
  void invalidate();

protected slots:
  void updateStatistics();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpStatisticsPanel);

private:
  QTE_DECLARE_PRIVATE(vpStatisticsPanel);
};

#endif
