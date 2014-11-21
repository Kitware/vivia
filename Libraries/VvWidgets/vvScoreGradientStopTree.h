/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvScoreGradientStopTree_h
#define __vvScoreGradientStopTree_h

#include <QTreeWidget>

#include <qtGlobal.h>

#include <vgExport.h>

#include "vvScoreGradient.h"

class vvScoreGradientStopTreePrivate;

//-----------------------------------------------------------------------------
class VV_WIDGETS_EXPORT vvScoreGradientStopTree : public QTreeWidget
{
  Q_OBJECT

public:
  explicit vvScoreGradientStopTree(QWidget* parent = 0);
  virtual ~vvScoreGradientStopTree();

  vvScoreGradient stops() const;

signals:
  void stopsChanged(vvScoreGradient);

public slots:
  void setStops(vvScoreGradient);

  void addStop();
  void removeSelectedStops();

protected slots:
  void recomputeStops();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvScoreGradientStopTree)

  virtual bool edit(const QModelIndex&, EditTrigger, QEvent*);

private:
  QTE_DECLARE_PRIVATE(vvScoreGradientStopTree)
  Q_DISABLE_COPY(vvScoreGradientStopTree)
};

#endif
