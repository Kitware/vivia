// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsContour.h"

#include <QComboBox>

QTE_IMPLEMENT_D_FUNC_SHARED(vsContour)

//-----------------------------------------------------------------------------
class vsContourData : public QSharedData
{
public:
  int Id;
  vsContour::Type Type;
  QString Name;
  QPolygonF Points;
};

//-----------------------------------------------------------------------------
bool vsContour::isLoopType(vsContour::Type t)
{
  return (t == vsContour::Filter || t == vsContour::Selector);
}

//-----------------------------------------------------------------------------
vsContour::vsContour(int id, Type type, QPolygonF points, QString name)
  : d_ptr(new vsContourData)
{
  QTE_D_MUTABLE(vsContour);
  d->Id = id;
  d->Type = type;
  d->Name = name;
  d->Points = points;
}

//-----------------------------------------------------------------------------
vsContour::vsContour(const vsContour& other)
  : d_ptr(other.d_ptr)
{
}

//-----------------------------------------------------------------------------
vsContour& vsContour::operator=(const vsContour& other)
{
  this->d_ptr = other.d_ptr;
  return *this;
}

//-----------------------------------------------------------------------------
vsContour::~vsContour()
{
}

//-----------------------------------------------------------------------------
void vsContour::populateTypeWidget(QComboBox* box)
{
  for (int i = vsContour::Annotation; i < vsContour::NoType; ++i)
    {
    vsContour::Type t = static_cast<vsContour::Type>(i);
    box->addItem(vsContour::typeString(t),
                 QVariant::fromValue<vsContour::Type>(t));
    }
}

//-----------------------------------------------------------------------------
QString vsContour::typeString(vsContour::Type type)
{
  switch (type)
    {
    case vsContour::Annotation:
      return "Annotation";
    case vsContour::Tripwire:
      return "Tripwire";
    case vsContour::Filter:
      return "Filter";
    case vsContour::Selector:
      return "Selector";
    default:
      return "(unknown)";
    }
}

//-----------------------------------------------------------------------------
int vsContour::id() const
{
  QTE_D_SHARED(vsContour);
  return d->Id;
}

//-----------------------------------------------------------------------------
vsContour::Type vsContour::type() const
{
  QTE_D_SHARED(vsContour);
  return d->Type;
}

//-----------------------------------------------------------------------------
QString vsContour::name() const
{
  QTE_D_SHARED(vsContour);
  return d->Name;
}

//-----------------------------------------------------------------------------
QPolygonF vsContour::points() const
{
  QTE_D_SHARED(vsContour);
  return d->Points;
}

//-----------------------------------------------------------------------------
void vsContour::setType(vsContour::Type type)
{
  QTE_D_MUTABLE(vsContour);
  d->Type = type;
}

//-----------------------------------------------------------------------------
void vsContour::setName(const QString& name)
{
  QTE_D_MUTABLE(vsContour);
  d->Name = name;
}

//-----------------------------------------------------------------------------
void vsContour::setPoints(const QPolygonF& points)
{
  QTE_D_MUTABLE(vsContour);
  d->Points = points;
}
