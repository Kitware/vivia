/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvCustomQueryServerChooser.h"
#include "ui_vvCustomQueryServerChooser.h"

#include <qtScopedValueChange.h>
#include <qtUtil.h>

QTE_IMPLEMENT_D_FUNC(vvCustomQueryServerChooser)

namespace { typedef QPair<QString, QString> QueryItem; }

//-----------------------------------------------------------------------------
class vvCustomQueryServerChooserPrivate
{
public:
  vvCustomQueryServerChooserPrivate() : blockUpdates(false) {}

  Ui::vvCustomQueryServerChooser UI;
  bool blockUpdates;
  QUrl uri;
};

//-----------------------------------------------------------------------------
vvCustomQueryServerChooser::vvCustomQueryServerChooser(QWidget* parent)
  : vvAbstractQueryServerChooser(parent),
    d_ptr(new vvCustomQueryServerChooserPrivate)
{
  QTE_D(vvCustomQueryServerChooser);
  d->UI.setupUi(this);

  connect(d->UI.scheme, SIGNAL(editingFinished()),
          this, SLOT(updateUri()));
  connect(d->UI.authority, SIGNAL(editingFinished()),
          this, SLOT(updateUri()));
  connect(d->UI.path, SIGNAL(editingFinished()),
          this, SLOT(updateUri()));

  connect(d->UI.arguments, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateArgumentButtons()));
  connect(d->UI.arguments, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(updateUri()));

  connect(d->UI.argumentsAdd, SIGNAL(clicked()),
          this, SLOT(addArgument()));
  connect(d->UI.argumentsRemove, SIGNAL(clicked()),
          this, SLOT(removeArgument()));
  connect(d->UI.argumentsMoveUp, SIGNAL(clicked()),
          this, SLOT(moveArgumentUp()));
  connect(d->UI.argumentsMoveDown, SIGNAL(clicked()),
          this, SLOT(moveArgumentDown()));
}

//-----------------------------------------------------------------------------
vvCustomQueryServerChooser::~vvCustomQueryServerChooser()
{
}

//-----------------------------------------------------------------------------
QUrl vvCustomQueryServerChooser::uri() const
{
  QTE_D_CONST(vvCustomQueryServerChooser);
  return d->uri;
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::setUri(QUrl newUri)
{
  QTE_D(vvCustomQueryServerChooser);

  // Block calls to updateUri() while we make changes to the UI
  d->blockUpdates = true;

  // Fill in UI fields
  d->UI.scheme->setText(newUri.scheme());
  d->UI.authority->setText(newUri.authority());
  d->UI.path->setText(newUri.path());
  d->UI.arguments->clear();
  foreach (QueryItem qi, newUri.queryItems())
    {
    this->addArgument(qi.first, qi.second);
    }

  // Recalculate URI from UI fields
  d->blockUpdates = false;
  this->updateUri();
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::updateUri()
{
  QTE_D(vvCustomQueryServerChooser);

  if (d->blockUpdates)
    {
    return;
    }

  qtScopedValueChange<bool> blockUpdates(d->blockUpdates, true);
  QUrl newUri = d->uri;

  // Validate and set scheme
  QString newScheme = d->UI.scheme->text();
  bool schemeIsValid = true;
  for (int i = 0, k = newScheme.size(); i < k; ++i)
    {
    QChar c = newScheme[i];
    if (c.unicode() > 127 || !c.isLetterOrNumber())
      {
      schemeIsValid = false;
      break;
      }
    }
  if (schemeIsValid)
    {
    newUri.setScheme(newScheme);
    }
  d->UI.scheme->setText(newUri.scheme());

  // Validate and set authority
  QString newAuthority = d->UI.authority->text();
  newUri.setAuthority(newAuthority);
  if (newUri.authority().isEmpty() && !newAuthority.isEmpty())
    {
    newUri.setAuthority(d->uri.authority());
    }
  d->UI.authority->setText(newUri.authority());

  // Set path
  QByteArray newPath = QUrl::toPercentEncoding(d->UI.path->text(), "/", "?");
  newUri.setEncodedPath(newPath);
  d->UI.path->setText(newUri.path());

  // Set query items
  QList<QueryItem> queryItems;
  foreach_child (QTreeWidgetItem* item, d->UI.arguments->invisibleRootItem())
    {
    queryItems.append(QueryItem(item->text(0), item->text(1)));
    }
  newUri.setQueryItems(queryItems);

  // Done
  d->uri = newUri;
  emit this->uriChanged(newUri);
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::setSelectedArgument(QTreeWidgetItem* item)
{
  QTE_D(vvCustomQueryServerChooser);
  foreach (QTreeWidgetItem* sitem, d->UI.arguments->selectedItems())
    {
    sitem->setSelected(sitem == item);
    }
  item->setSelected(true);
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::addArgument(QString name, QString value)
{
  QTE_D(vvCustomQueryServerChooser);
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setText(1, value);
  d->UI.arguments->addTopLevelItem(item);
  if (name.isEmpty())
    {
    item->setText(0, "arg");
    this->setSelectedArgument(item);
    d->UI.arguments->editItem(item, 0);
    }
  else
    {
    item->setText(0, name);
    }
  this->updateArgumentButtons();
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::removeArgument()
{
  QTE_D(vvCustomQueryServerChooser);
  foreach (QTreeWidgetItem* item, d->UI.arguments->selectedItems())
    {
    d->UI.arguments->invisibleRootItem()->removeChild(item);
    }
  this->updateUri();
  this->updateArgumentButtons();
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::moveArgumentUp()
{
  QTE_D(vvCustomQueryServerChooser);
  QTreeWidgetItem* item = d->UI.arguments->selectedItems().value(0, 0);
  QTreeWidgetItem* root = d->UI.arguments->invisibleRootItem();

  int i = root->indexOfChild(item);
  if (item && i > 0)
    {
    item = root->takeChild(i);
    root->insertChild(i - 1, item);
    this->updateUri();
    this->setSelectedArgument(item);
    this->updateArgumentButtons();
    }
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::moveArgumentDown()
{
  QTE_D(vvCustomQueryServerChooser);
  QTreeWidgetItem* item = d->UI.arguments->selectedItems().value(0, 0);
  QTreeWidgetItem* root = d->UI.arguments->invisibleRootItem();

  int i = root->indexOfChild(item);
  if (item && i >= 0)
    {
    item = root->takeChild(i);
    root->insertChild(i + 1, item);
    this->updateUri();
    this->setSelectedArgument(item);
    this->updateArgumentButtons();
    }
}

//-----------------------------------------------------------------------------
void vvCustomQueryServerChooser::updateArgumentButtons()
{
  QTE_D(vvCustomQueryServerChooser);
  QTreeWidget* tree = d->UI.arguments;
  QList<QTreeWidgetItem*> selection = tree->selectedItems();

  bool enableRemove = !selection.isEmpty();
  bool enableMoveUp = !selection.isEmpty();
  bool enableMoveDown = !selection.isEmpty();
  if (!selection.isEmpty())
    {
    int k = tree->topLevelItemCount() - 1;
    enableMoveUp = !tree->isItemSelected(tree->topLevelItem(0));
    enableMoveDown = !tree->isItemSelected(tree->topLevelItem(k));
    }

  d->UI.argumentsRemove->setEnabled(enableRemove);
  d->UI.argumentsMoveUp->setEnabled(enableMoveUp);
  d->UI.argumentsMoveDown->setEnabled(enableMoveDown);
}
