/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsContour_h
#define __vsContour_h

#include <QPolygonF>
#include <QSharedDataPointer>
#include <QVariant>

#include <qtGlobal.h>

#include <vgExport.h>

class QComboBox;

class vsContourData;

class VSP_DATA_EXPORT vsContour
{
public:
  enum Type
    {
    Annotation,
    Tripwire,
    Filter,
    Selector,
    NoType // keep last
    };

  static bool isLoopType(Type);

  vsContour(int id = -1, Type type = NoType, QPolygonF points = QPolygonF(),
            QString name = QString());
  vsContour(const vsContour&);
  ~vsContour();

  vsContour& operator=(const vsContour&);

  static void populateTypeWidget(QComboBox*);
  static QString typeString(Type);

  int id() const;
  Type type() const;
  QString name() const;
  QPolygonF points() const;

  void setType(Type);
  void setName(const QString&);
  void setPoints(const QPolygonF&);

protected:
  QTE_DECLARE_SHARED_PTR(vsContour)

private:
  QTE_DECLARE_SHARED(vsContour)
};

Q_DECLARE_METATYPE(vsContour::Type)

#endif
