/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfDataSource.h"

#include "vdfDataSourceInterface.h"

#include <QDebug>
#include <QSet>

//-----------------------------------------------------------------------------
class vdfDataSourcePrivate
{
public:
  vdfDataSourcePrivate();

  vdfDataSource::Status Status;

  QObjectList Interfaces;
  QSet<const QMetaObject*> InterfaceTypes;
};

QTE_IMPLEMENT_D_FUNC(vdfDataSource)

//-----------------------------------------------------------------------------
vdfDataSourcePrivate::vdfDataSourcePrivate() :
  Status(vdfDataSource::Unstarted)
{
}

//-----------------------------------------------------------------------------
vdfDataSource::vdfDataSource(QObject* parent) :
  vdfNodeBase(parent), d_ptr(new vdfDataSourcePrivate)
{
}

//-----------------------------------------------------------------------------
vdfDataSource::~vdfDataSource()
{
}

//-----------------------------------------------------------------------------
QString vdfDataSource::type() const
{
  QTE_THREADSAFE_STATIC_CONST(QString) myType("Data Source");
  return myType;
}

//-----------------------------------------------------------------------------
bool vdfDataSource::canUpdate() const
{
  return false;
}

//-----------------------------------------------------------------------------
vdfDataSource::Mechanism vdfDataSource::mechanism() const
{
  return vdfDataSource::None;
}

//-----------------------------------------------------------------------------
vdfDataSource::Status vdfDataSource::status() const
{
  QTE_D_CONST(vdfDataSource);
  return d->Status;
}

//-----------------------------------------------------------------------------
void vdfDataSource::setStatus(vdfDataSource::Status newStatus)
{
  QTE_D(vdfDataSource);
  if (d->Status != newStatus)
    {
    d->Status = newStatus;
    emit this->statusChanged(newStatus);
    }
}

//-----------------------------------------------------------------------------
QObjectList vdfDataSource::interfaces() const
{
  QTE_D_CONST(vdfDataSource);
  return d->Interfaces;
}

//-----------------------------------------------------------------------------
QSet<const QMetaObject*> vdfDataSource::interfaceTypes() const
{
  QTE_D_CONST(vdfDataSource);
  return d->InterfaceTypes;
}

//-----------------------------------------------------------------------------
bool vdfDataSource::hasInterface(const QMetaObject* type) const
{
  QTE_D_CONST(vdfDataSource);
  return d->InterfaceTypes.contains(type);
}

//-----------------------------------------------------------------------------
QObject* vdfDataSource::interface(const QMetaObject* type) const
{
  // Iterate over available interfaces; we could do this more efficiently if
  // we built a map of interfaces by type, but we don't expect this to be
  // called from performance sensitive code, and also don't expect that most
  // data sources will implement more than a few interfaces
  foreach (QObject* const i, this->interfaces())
    {
    // Check if interface can be casted to the specified type
    if (type->cast(i))
      {
      // If yes, return the interface
      return i;
      }
    }

  // No suitable interface found
  return 0;
}

//-----------------------------------------------------------------------------
void vdfDataSource::addInterface(
  QObject* interface, vdfDataSourceInterface* checkType)
{
  // checkType is passed as a compile-time check that the interface is a
  // vdfDataSourceWrapper, and isn't otherwise used
  Q_UNUSED(checkType)

  QTE_D(vdfDataSource);
  for (const QMetaObject* type = interface->metaObject();
       type && type != &QObject::staticMetaObject; type = type->superClass())
    {
    if (d->InterfaceTypes.contains(type))
      {
      QString myType = QString("(%1)").arg(this->metaObject()->className());

      qCritical() << "vdfDataSource::addInterface(): error:"
                  << "cannot add an interface of type"
                  << interface->metaObject()->className()
                  << "to" << qPrintable(myType) << this->displayName();
      qCritical() << "vdfDataSource::addInterface(): error:"
                  << "an interface of type" << type->className()
                  << "has already been added";
      qFatal(0);
      }
    d->InterfaceTypes.insert(type);
    }
  d->Interfaces.append(interface);
}
