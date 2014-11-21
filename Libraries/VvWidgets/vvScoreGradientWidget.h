/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvScoreGradientWidget_h
#define __vvScoreGradientWidget_h

#include <QWidget>

#include <qtGlobal.h>
#include <qtGradient.h>

#include <vgExport.h>

#include "vvScoreGradient.h"

class vvScoreGradientWidgetPrivate;

class VV_WIDGETS_EXPORT vvScoreGradientWidget : public QWidget
{
  Q_OBJECT

public:
  explicit vvScoreGradientWidget(QWidget* parent = 0);
  virtual ~vvScoreGradientWidget();

  qtGradient gradient() const;

  vvScoreGradient stops() const;

  qtGradient::InterpolationMode interpolationMode() const;
  void setInterpolationMode(qtGradient::InterpolationMode);

signals:
  void stopsChanged(vvScoreGradient);

public slots:
  void setStops(vvScoreGradient);

protected slots:
  void updatePreview();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvScoreGradientWidget)

private:
  QTE_DECLARE_PRIVATE(vvScoreGradientWidget)
  Q_DISABLE_COPY(vvScoreGradientWidget)
};

#endif
