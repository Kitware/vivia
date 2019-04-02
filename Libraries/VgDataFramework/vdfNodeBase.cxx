/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfNodeBase.h"

#include "vdfNodeProxy.h"
#include "vdfSelector.h"
#include "vdfSelectorSet.h"

#include <QChildEvent>
#include <QSet>
#include <QPointer>

//-----------------------------------------------------------------------------
class vdfNodeBasePrivate
{
public:
  vdfNodeBasePrivate() :
    DisplayName("Unnamed Node"),
    Visibility(true),
    ReferenceCount(0)
    {}

  QString DisplayName;
  bool Visibility;
  int ReferenceCount;

  QSet<vdfNodeProxy*> Connections;

  QList<QPointer<QObject>> NewChildren;
};

QTE_IMPLEMENT_D_FUNC(vdfNodeBase)

//-----------------------------------------------------------------------------
vdfNodeBase::vdfNodeBase(QObject* parent) :
  QObject(parent), d_ptr(new vdfNodeBasePrivate)
{
  vdf::registerMetaTypes();
}

//-----------------------------------------------------------------------------
vdfNodeBase::~vdfNodeBase()
{
}

//-----------------------------------------------------------------------------
const QString vdfNodeBase::displayName() const
{
  QTE_D_CONST(vdfNodeBase);
  return d->DisplayName;
}

//-----------------------------------------------------------------------------
void vdfNodeBase::setDisplayName(const QString& value)
{
  QTE_D(vdfNodeBase);
  if (d->DisplayName != value)
    {
    d->DisplayName = value;
    emit this->displayNameChanged(value);
    }
}

//-----------------------------------------------------------------------------
QString vdfNodeBase::type() const
{
  QTE_THREADSAFE_STATIC_CONST(QString) myType("Collection");
  return myType;
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::isHidden() const
{
  return false;
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::isVisible() const
{
  QTE_D_CONST(vdfNodeBase);
  return d->Visibility;
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::setVisible(bool visibility)
{
  if (!this->canChangeVisibility())
    {
    // Changing node visibility not allowed; do nothing and return failure
    return false;
    }

  QTE_D(vdfNodeBase);
  if (d->Visibility != visibility)
    {
    d->Visibility = visibility;
    emit this->visibilityChanged(visibility);
    }

  // Visibility state is now as requested; return success (even if the state
  // didn't actually change, the operation was still allowed)
  return true;
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::canChangeVisibility() const
{
  return true;
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::canUpdate() const
{
  return true;
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::canDelete() const
{
  return true;
}

//-----------------------------------------------------------------------------
QSet<vdfSelectorType> vdfNodeBase::supportedSelectors() const
{
  return QSet<vdfSelectorType>();
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::isSelectorSupported(const vdfSelectorType& st) const
{
  return this->supportedSelectors().contains(st);
}

//-----------------------------------------------------------------------------
bool vdfNodeBase::isSelectorSupported(const vdfSelector* selector) const
{
  return this->isSelectorSupported(selector->type());
}

//-----------------------------------------------------------------------------
vdfNodeProxy* vdfNodeBase::connect(QObject* consumer)
{
  if (this->canUpdate())
    {
    QTE_D(vdfNodeBase);

    vdfNodeProxy* const connection = new vdfNodeProxy(this);
    connect(consumer, SIGNAL(destroyed()), connection, SLOT(deleteLater()));
    this->enter();

    d->Connections.insert(connection);
    return connection;
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vdfNodeBase::disconnect(vdfNodeProxy* connection)
{
  QTE_D(vdfNodeBase);
  if (d->Connections.contains(connection))
    {
    // Because the connection calls this method when it is being destroyed, we
    // cannot just delete the connection, but we still want to ensure that it
    // goes away...
    connection->deleteLater();
    d->Connections.remove(connection);
    this->leave();
    }
}

//-----------------------------------------------------------------------------
void vdfNodeBase::enter()
{
  QTE_D(vdfNodeBase);

  Q_ASSERT(d->ReferenceCount >= 0);

  if (d->ReferenceCount <= 0)
    {
    // Acquire node when first user enters
    this->acquire();
    }

  // Increment reference count
  ++d->ReferenceCount;
}

//-----------------------------------------------------------------------------
void vdfNodeBase::leave()
{
  QTE_D(vdfNodeBase);

  // Decrement reference count
  --d->ReferenceCount;

  if (d->ReferenceCount <= 0)
    {
    // Release node after last user leaves
    this->release();
    }

  Q_ASSERT(d->ReferenceCount >= 0);
}

//-----------------------------------------------------------------------------
void vdfNodeBase::acquire()
{
}

//-----------------------------------------------------------------------------
void vdfNodeBase::release()
{
}

//-----------------------------------------------------------------------------
void vdfNodeBase::select(const vdfNodeProxy* connection)
{
  Q_UNUSED(connection)
}

//-----------------------------------------------------------------------------
void vdfNodeBase::update(
  const vdfNodeProxy* connection, qint64 requestId,
  const vdfSelectorSet& selectors, vdf::UpdateFlags mode)
{
  Q_UNUSED(selectors)
  Q_UNUSED(mode)

  // Base class has no data, so cannot update
  emit connection->progress(vdfNodeProxy::UpdateDiscarded, requestId);
}

//-----------------------------------------------------------------------------
void vdfNodeBase::addChild(vdfNodeBase* child)
{
  Q_UNUSED(child)
}

//-----------------------------------------------------------------------------
/// \cond internal
void vdfNodeBase::childEvent(QChildEvent* e)
{
  if (e->added())
    {
    QTE_D(vdfNodeBase);

    if (d->NewChildren.isEmpty())
      {
      // For the first new child we see in a given iteration of the event loop,
      // schedule a call to process new children
      QMetaObject::invokeMethod(this, "updateChildren", Qt::QueuedConnection);
      }

    // Add new child to new children list so we can process it later, once it
    // is fully constructed
    d->NewChildren.append(e->child());
    }

  QObject::childEvent(e);
}


//-----------------------------------------------------------------------------
void vdfNodeBase::updateChildren()
{
  QTE_D(vdfNodeBase);
  foreach (const auto& child, d->NewChildren)
    {
    vdfNodeBase* const childNode = qobject_cast<vdfNodeBase*>(child.data());
    if (childNode)
      {
      // If the cast succeeded (which it will only for actual nodes that have
      // not been deleted by the time we come back around the event loop),
      // process the new child
      this->addChild(childNode);
      }
    }
}
/// \endcond
