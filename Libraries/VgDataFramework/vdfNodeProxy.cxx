/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vdfNodeProxy.h"

#include "vdfNodeBase.h"

//-----------------------------------------------------------------------------
class vdfNodeProxyPrivate
{
public:
  vdfNodeProxyPrivate(vdfNodeBase* node) : Node(node) {}

  vdf::UpdateFlags UpdateMode;
  vdfNodeBase* const Node;
};

QTE_IMPLEMENT_D_FUNC(vdfNodeProxy)

//-----------------------------------------------------------------------------
/// \cond internal
vdfNodeProxy::vdfNodeProxy(vdfNodeBase* node) :
  QObject(node), d_ptr(new vdfNodeProxyPrivate(node))
{
}
/// \endcond

//-----------------------------------------------------------------------------
vdfNodeProxy::~vdfNodeProxy()
{
  QTE_D(vdfNodeProxy);
  d->Node->disconnect(this);
}

//-----------------------------------------------------------------------------
vdfNodeBase* vdfNodeProxy::node() const
{
  QTE_D_CONST(vdfNodeProxy);
  return d->Node;
}

//-----------------------------------------------------------------------------
vdf::UpdateFlags vdfNodeProxy::updateMode() const
{
  QTE_D_CONST(vdfNodeProxy);
  return d->UpdateMode;
}

//-----------------------------------------------------------------------------
void vdfNodeProxy::setUpdateMode(vdf::UpdateFlags mode)
{
  QTE_D(vdfNodeProxy);
  d->UpdateMode = mode;
}

//-----------------------------------------------------------------------------
void vdfNodeProxy::makeCurrent() const
{
  QTE_D_CONST(vdfNodeProxy);
  d->Node->select(this);
}

//-----------------------------------------------------------------------------
void vdfNodeProxy::update(const vdfSelectorSet& selectors,
                          qint64 requestId) const
{
  QTE_D_CONST(vdfNodeProxy);
  d->Node->update(this, requestId, selectors, d->UpdateMode);
}
