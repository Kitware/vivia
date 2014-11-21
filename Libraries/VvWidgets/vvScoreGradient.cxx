/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvScoreGradient.h"

#include <qtGradient.h>

QTE_IMPLEMENT_D_FUNC(vvScoreGradient)

namespace // anonymous
{

//-----------------------------------------------------------------------------
qreal position(qreal previousThreshold, qreal currentThreshold, bool terminal)
{
  if (terminal)
    {
    return 1.0 - currentThreshold;
    }
  return 0.5 * ((1.0 - previousThreshold) + (1.0 - currentThreshold));
}

//-----------------------------------------------------------------------------
qreal weight(qreal currentPosition, qreal nextPosition, qreal threshold)
{
  const qreal range = (nextPosition - currentPosition);
  return ((1.0 - threshold) - currentPosition) / range;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
bool operator==(
  const vvScoreGradient::Stop& a, const vvScoreGradient::Stop& b)
{
  return (a.threshold == b.threshold) &&
         (a.color == b.color) &&
         (a.text == b.text);
}

//-----------------------------------------------------------------------------
bool operator<(
  const vvScoreGradient::Stop& a, const vvScoreGradient::Stop& b)
{
  if (a.threshold > b.threshold)
    {
    return true;
    }
  if (a.threshold < b.threshold)
    {
    return false;
    }
  if (a.color.rgba() > b.color.rgba())
    {
    return true;
    }
  if (a.color.rgba() < b.color.rgba())
    {
    return false;
    }
  return QString::localeAwareCompare(a.text, b.text);
}

//-----------------------------------------------------------------------------
class vvScoreGradientPrivate
{
public:
  QList<vvScoreGradient::Stop> stops;
};

//-----------------------------------------------------------------------------
vvScoreGradient::Stop::Stop() : threshold(0.0)
{
}

//-----------------------------------------------------------------------------
vvScoreGradient::Stop::Stop(
  const QString& text, const QColor& color, qreal threshold) :
  text(text),
  color(color),
  threshold(threshold)
{
}

//-----------------------------------------------------------------------------
vvScoreGradient::vvScoreGradient() :
  d_ptr(new vvScoreGradientPrivate)
{
  QTE_D(vvScoreGradient);
  d->stops.append(Stop("Confirmed", Qt::green, 0.97));
  d->stops.append(Stop("Probable", Qt::yellow, 0.34));
  d->stops.append(Stop("Possible", Qt::red, 0.0));
  qSort(d->stops);
}

//-----------------------------------------------------------------------------
vvScoreGradient::~vvScoreGradient()
{
}

//-----------------------------------------------------------------------------
vvScoreGradient::vvScoreGradient(const QList<vvScoreGradient::Stop>& stops) :
  d_ptr(new vvScoreGradientPrivate)
{
  QTE_D(vvScoreGradient);
  d->stops = stops;
  qSort(d->stops);
}

//-----------------------------------------------------------------------------
vvScoreGradient::vvScoreGradient(const vvScoreGradient& other) :
  d_ptr(new vvScoreGradientPrivate)
{
  QTE_D(vvScoreGradient);
  d->stops = other.d_func()->stops;
}

//-----------------------------------------------------------------------------
vvScoreGradient& vvScoreGradient::operator=(const vvScoreGradient& other)
{
  QTE_D(vvScoreGradient);
  d->stops = other.d_func()->stops;
  return *this;
}

//-----------------------------------------------------------------------------
bool vvScoreGradient::operator==(const vvScoreGradient& other) const
{
  return this->stops() == other.stops();
}

//-----------------------------------------------------------------------------
bool vvScoreGradient::operator!=(const vvScoreGradient& other) const
{
  return this->stops() != other.stops();
}

//-----------------------------------------------------------------------------
qtGradient vvScoreGradient::gradient(qtGradient::InterpolationMode im) const
{
  QTE_D_CONST(vvScoreGradient);
  const QList<Stop>& stops = d->stops;

  qtGradient result;
  result.setInterpolationMode(im);

  // Handle edge cases (no stops, only one stop)
  if (stops.empty())
    {
    return result;
    }
  else if (stops.count() == 1)
    {
    const QColor c = stops[0].color;
    result.insertStop(0.0, c);
    result.insertStop(1.0, c);
    return result;
    }

  // Insert first stop
  qreal t = stops[0].threshold, nt = stops[1].threshold;
  qreal p = 0.0, np = position(t, nt, stops.count() == 2);
  result.insertStop(p, stops[0].color, weight(p, np, t));

  // Insert intermediary stops
  for (int i = 1, k = stops.count() - 1; i < k; ++i)
    {
    t = stops[i].threshold; nt = stops[i + 1].threshold;
    p = np; np = position(t, nt, i + 1 >= k);
    result.insertStop(p, stops[i].color, weight(p, np, t));
    }

  // Insert final stop
  result.insertStop(1.0, stops.last().color);

  return result;
}

//-----------------------------------------------------------------------------
QList<vvScoreGradient::Stop> vvScoreGradient::stops() const
{
  QTE_D_CONST(vvScoreGradient);
  return d->stops;
}

//-----------------------------------------------------------------------------
void vvScoreGradient::setStops(const QList<vvScoreGradient::Stop>& stops)
{
  *this = vvScoreGradient(stops);
}
