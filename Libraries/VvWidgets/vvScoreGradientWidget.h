// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
