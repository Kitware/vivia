// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvScoreGradient_h
#define __vvScoreGradient_h

#include <QColor>
#include <QString>
#include <QList>
#include <QMetaType>

#include <qtGlobal.h>
#include <qtGradient.h>

#include <vgExport.h>

class vvScoreGradientPrivate;

class VV_WIDGETS_EXPORT vvScoreGradient
{
public:
  struct Stop
    {
    Stop();
    Stop(const QString& text, const QColor& color, qreal threshold);

    QString text;
    QColor color;
    qreal threshold;
    };

  explicit vvScoreGradient();
  virtual ~vvScoreGradient();

  vvScoreGradient(const QList<Stop>&);
  vvScoreGradient(const vvScoreGradient&);
  vvScoreGradient& operator=(const vvScoreGradient&);

  bool operator==(const vvScoreGradient&) const;
  bool operator!=(const vvScoreGradient&) const;

  qtGradient gradient(qtGradient::InterpolationMode = 0) const;

  QList<Stop> stops() const;
  void setStops(const QList<Stop>&);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvScoreGradient)

private:
  QTE_DECLARE_PRIVATE(vvScoreGradient)
};

Q_DECLARE_METATYPE(vvScoreGradient)

#endif
