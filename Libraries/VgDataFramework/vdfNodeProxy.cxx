// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
