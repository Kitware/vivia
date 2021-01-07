// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqConfigureDialog.h"

#include <QSettings>
#include <QUrl>

#include <qtUtil.h>

#include <vgFileDialog.h>

#include <vvQueryService.h>

#include <vvDescriptorStyleDelegate.h>
#include <vvQueryServerDialog.h>

#include "vqSettings.h"

//-----------------------------------------------------------------------------
vqConfigureDialog::vqConfigureDialog(QWidget* parent) : QDialog(parent)
{
  this->settings_ = new vqSettings;
  this->UI.setupUi(this);
  qtUtil::setStandardIcons(this->UI.buttons);
  this->UI.pageChooser->item(0)->setSelected(true);

  // Data Sources page
  connect(this->UI.queryServerUri, SIGNAL(textChanged(QString)),
          this, SLOT(queryServerUriChanged(QString)));
  connect(this->UI.queryVideoUri, SIGNAL(textChanged(QString)),
          this, SLOT(queryVideoUriChanged(QString)));
  connect(this->UI.queryServerUriEdit, SIGNAL(clicked()),
          this, SLOT(showQueryServerEditUriDialog()));
  connect(this->UI.queryVideoUriPickLocalDir, SIGNAL(clicked()),
          this, SLOT(showQueryVideoPickLocalDirDialog()));
  connect(this->UI.videoProviderAddLocalArchive, SIGNAL(clicked()),
          this, SLOT(showVideoProviderAddLocalArchiveDialog()));
  connect(this->UI.videoProviders, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateVideoProviderButtons()));
  connect(this->UI.videoProviderUp, SIGNAL(clicked()),
          this, SLOT(videoProvidersSelectionMoveUp()));
  connect(this->UI.videoProviderDown, SIGNAL(clicked()),
          this, SLOT(videoProvidersSelectionMoveDown()));
  connect(this->UI.videoProviderDelete, SIGNAL(clicked()),
          this, SLOT(videoProvidersSelectionDelete()));

  // Display page
  connect(this->UI.resultsPerPage, SIGNAL(valueChanged(int)),
          this, SLOT(resultsPerPageChanged(int)));
  connect(this->UI.resultClipPadding, SIGNAL(valueChanged(double)),
          this, SLOT(resultClipPaddingChanged(double)));
  connect(this->UI.resultColoring, SIGNAL(stopsChanged(vvScoreGradient)),
          this, SLOT(resultColoringChanged(vvScoreGradient)));

  // QF page
  connect(this->UI.queryCacheUri, SIGNAL(textChanged(QString)),
          this, SLOT(queryCacheUriChanged(QString)));
  connect(this->UI.queryCacheUriPickLocalDir, SIGNAL(clicked()),
          this, SLOT(showQueryCachePickLocalDirDialog()));
  connect(this->UI.predefinedQueryUriPickLocalDir, SIGNAL(clicked()),
          this, SLOT(showPredefinedQueryPickLocalDirDialog()));
  connect(this->UI.predefinedQueryUri, SIGNAL(textChanged(QString)),
          this, SLOT(predefinedQueryUriChanged(QString)));
  connect(this->UI.descriptors, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateDescriptorButtons()));
  connect(this->UI.descriptors, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(descriptorsChanged()));
  connect(this->UI.addDescriptor, SIGNAL(clicked()),
          this, SLOT(addDescriptor()));
  connect(this->UI.removeDescriptors, SIGNAL(clicked()),
          this, SLOT(removeDescriptors()));

  // IQR page
  connect(this->UI.iqrWorkingSetSize, SIGNAL(valueChanged(int)),
          this, SLOT(iqrWorkingSetSizeChanged(int)));
  connect(this->UI.iqrRefinementSetSize, SIGNAL(valueChanged(int)),
          this, SLOT(iqrRefinementSetSizeChanged(int)));

  vvDescriptorStyleDelegate* delegate = new vvDescriptorStyleDelegate;
  this->UI.descriptors->setItemDelegateForColumn(1, delegate);

  // Dialog
  connect(this->UI.buttons->button(QDialogButtonBox::Apply),
          SIGNAL(pressed()), this, SLOT(apply()));
  connect(this->UI.buttons->button(QDialogButtonBox::Reset),
          SIGNAL(pressed()), this, SLOT(reset()));

  this->reset();
}

//-----------------------------------------------------------------------------
vqConfigureDialog::~vqConfigureDialog()
{
  QAbstractItemDelegate* delegate =
    this->UI.descriptors->itemDelegateForColumn(1);
  this->UI.descriptors->setItemDelegateForColumn(1, 0);
  delete delegate;

  delete this->settings_;
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::accept()
{
  this->apply();
  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::reject()
{
  if (this->settings_->wasCommitted())
    QDialog::accept();
  else
    QDialog::reject();
}

//-----------------------------------------------------------------------------
#define MARK_CHANGED(_n) \
  const bool _n##Modified = vqSettings()._n() != this->settings_->_n()
#define EMIT_CHANGED(_n) \
  if (_n##Modified) emit this->_n##Changed()
void vqConfigureDialog::apply()
{
  // Mark changed settings so we know what to emit later
  MARK_CHANGED(scoreGradient);
  MARK_CHANGED(predefinedQueryUri);

  // Commit new settings and clear modified flag
  this->settings_->commit();
  this->setModified();

  // Emit notification signals for settings that changed
  EMIT_CHANGED(scoreGradient);
  EMIT_CHANGED(predefinedQueryUri);
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::reset()
{
  this->settings_->discard();
  this->UI.queryServerUri->setText(
    this->settings_->queryServerUri().toString());
  this->UI.queryVideoUri->setText(
    this->settings_->queryVideoUri().toString());
  this->UI.queryCacheUri->setText(
    this->settings_->queryCacheUri().toString());
  this->UI.videoProviders->clear();
  foreach (QUrl uri, this->settings_->videoProviders())
    this->UI.videoProviders->addItem(uri.toString());
  this->UI.predefinedQueryUri->setText(
    this->settings_->predefinedQueryUri().toString());
  this->UI.resultsPerPage->setValue(
    this->settings_->resultPageCount());
  this->UI.resultClipPadding->setValue(
    this->settings_->resultClipPadding());
  this->UI.resultColoring->setStops(
    this->settings_->scoreGradient());
  this->UI.iqrWorkingSetSize->setValue(
    this->settings_->iqrWorkingSetSize());
  this->UI.iqrRefinementSetSize->setValue(
    this->settings_->iqrRefinementSetSize());

  this->UI.descriptors->clear();
  vvDescriptorStyle::Map groupings = this->settings_->descriptorStyles();
  foreach_iter (vvDescriptorStyle::Map::const_iterator, iter, groupings)
    {
    QStringList sl;
    sl.append(iter.key());
    sl.append(vvDescriptorStyle::string(iter.value()));
    QVariant data =
      QVariant::fromValue<vvDescriptorStyle::Styles>(iter.value());
    QTreeWidgetItem* item = new QTreeWidgetItem(sl);
    item->setData(1, Qt::UserRole, data);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    this->UI.descriptors->addTopLevelItem(item);
    }
  this->UI.descriptors->sortByColumn(0, Qt::AscendingOrder);
  qtUtil::resizeColumnsToContents(this->UI.descriptors);

  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::setModified()
{
  bool modified = this->settings_->hasUncommittedChanges();
  this->UI.buttons->button(QDialogButtonBox::Apply)->setEnabled(modified);
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::queryServerUriChanged(QString newUri)
{
  this->settings_->setQueryServerUri(newUri);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::showQueryServerEditUriDialog()
{
  vvQueryServerDialog dialog;
  vvQueryService::registerChoosers(&dialog);
  int result = dialog.exec(this->settings_->queryServerUri());
  if (result == QDialog::Accepted)
    this->UI.queryServerUri->setText(dialog.uri().toString());
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::queryVideoUriChanged(QString newUri)
{
  this->settings_->setQueryVideoUri(newUri);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::predefinedQueryUriChanged(QString newUri)
{
  this->settings_->setPredefinedQueryUri(newUri);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::showQueryVideoPickLocalDirDialog()
{
  QString path = vgFileDialog::getExistingDirectory(
                   this, "Local Directory for Query Videos...",
                   this->settings_->queryVideoUri().toLocalFile());
  if (!path.isEmpty())
    this->UI.queryVideoUri->setText(QUrl::fromLocalFile(path).toString());
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::showPredefinedQueryPickLocalDirDialog()
{
  QString path = vgFileDialog::getExistingDirectory(
                   this, "Local Directory for Predefined Queries...",
                   this->settings_->predefinedQueryUri().toLocalFile());
  if (!path.isEmpty())
    {
    this->UI.predefinedQueryUri->setText(
      QUrl::fromLocalFile(path).toString());
    }
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::queryCacheUriChanged(QString newUri)
{
  this->settings_->setQueryCacheUri(newUri);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::showQueryCachePickLocalDirDialog()
{
  QString path = vgFileDialog::getExistingDirectory(
                   this, "Local Directory for Query Cache...",
                   this->settings_->queryCacheUri().toLocalFile());
  if (!path.isEmpty())
    this->UI.queryCacheUri->setText(QUrl::fromLocalFile(path).toString());
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::showVideoProviderAddLocalArchiveDialog()
{
  QStringList fileNames = vgFileDialog::getOpenFileNames(
                            this, "Video provider to add...", QString(),
                            "KWA Video (*.index *.archive);;"
                            "KWA Video Indices (*.index);;"
                            "KWA Video Archives (*.archive);;"
                            "All files (*)");
  if (!fileNames.isEmpty())
    {
    foreach (QString fileName, fileNames)
      {
      QUrl uri = QUrl::fromLocalFile(fileName);
      this->UI.videoProviders->addItem(uri.toString());
      }
    this->settings_->setVideoProviders(this->videoProviders());
    this->setModified();
    }
}

//-----------------------------------------------------------------------------
QList<QUrl> vqConfigureDialog::videoProviders()
{
  QList<QUrl> providers;
  for (int n = 0; n < this->UI.videoProviders->count(); ++n)
    providers.append(QUrl(this->UI.videoProviders->item(n)->text()));
  return providers;
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::updateVideoProviderButtons()
{
  QListWidget* list = this->UI.videoProviders;
  QList<QListWidgetItem*> selection = list->selectedItems();

  bool enableUp = !selection.isEmpty();
  bool enableDown = !selection.isEmpty();
  bool enableDelete = !selection.isEmpty();
  if (!selection.isEmpty())
    {
    enableUp = !list->isItemSelected(list->item(0));
    enableDown = !list->isItemSelected(list->item(list->count() - 1));
    }

  this->UI.videoProviderUp->setEnabled(enableUp);
  this->UI.videoProviderDown->setEnabled(enableDown);
  this->UI.videoProviderDelete->setEnabled(enableDelete);
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::videoProvidersSelectionMoveUp()
{
  QListWidget* list = this->UI.videoProviders;
  QList<QListWidgetItem*> selection = list->selectedItems();

  if (selection.isEmpty())
    return;
  if (list->isItemSelected(list->item(0)))
    return;

  foreach (QListWidgetItem* item, list->selectedItems())
    {
    int row = list->row(item);
    item = list->takeItem(row);
    list->insertItem(row - 1, item);
    }
  foreach (QListWidgetItem* item, selection)
    list->setItemSelected(item, true);

  this->settings_->setVideoProviders(this->videoProviders());
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::videoProvidersSelectionMoveDown()
{
  QListWidget* list = this->UI.videoProviders;
  QList<QListWidgetItem*> selection = list->selectedItems();

  if (selection.isEmpty())
    return;
  if (list->isItemSelected(list->item(list->count() - 1)))
    return;

  int n = selection.count();
  while (n--)
    {
    int row = list->row(selection[n]);
    QListWidgetItem* item = list->takeItem(row);
    list->insertItem(row + 1, item);
    }
  foreach (QListWidgetItem* item, selection)
    list->setItemSelected(item, true);

  this->settings_->setVideoProviders(this->videoProviders());
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::videoProvidersSelectionDelete()
{
  QListWidget* list = this->UI.videoProviders;
  QList<QListWidgetItem*> selection = list->selectedItems();

  foreach (QListWidgetItem* item, list->selectedItems())
    delete list->takeItem(list->row(item));

  this->settings_->setVideoProviders(this->videoProviders());
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::resultsPerPageChanged(int newValue)
{
  this->settings_->setResultPageCount(newValue);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::resultClipPaddingChanged(double newValue)
{
  this->settings_->setResultClipPadding(newValue);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::resultColoringChanged(vvScoreGradient newGradient)
{
  this->settings_->setScoreGradient(newGradient);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::iqrWorkingSetSizeChanged(int newSize)
{
  this->settings_->setIqrWorkingSetSize(newSize);
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::iqrRefinementSetSizeChanged(int newSize)
{
  this->settings_->setIqrRefinementSetSize(newSize);
  this->setModified();
}

//-----------------------------------------------------------------------------
vvDescriptorStyle::Map vqConfigureDialog::descriptorStyles()
{
  vvDescriptorStyle::Map styles;
  for (int n = 0; n < this->UI.descriptors->topLevelItemCount(); ++n)
    {
    QString key = this->UI.descriptors->topLevelItem(n)->text(0);
    QVariant data =
      this->UI.descriptors->topLevelItem(n)->data(1, Qt::UserRole);
    styles.insert(key, data.value<vvDescriptorStyle::Styles>());
    }
  return styles;
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::updateDescriptorButtons()
{
  QList<QTreeWidgetItem*> selection = this->UI.descriptors->selectedItems();
  this->UI.removeDescriptors->setEnabled(!selection.isEmpty());
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::descriptorsChanged()
{
  this->settings_->setDescriptorStyles(this->descriptorStyles());
  this->setModified();
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::addDescriptor()
{
  // Create new tree item
  QStringList sl;
  sl.append("Descriptor");
  sl.append("All");

  QTreeWidgetItem* item = new QTreeWidgetItem(sl);
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  QVariant data = // FIXME shouldn't use all, maybe need DefaultStyle?
    QVariant::fromValue<vvDescriptorStyle::Styles>(vvDescriptorStyle::All);
  item->setData(1, Qt::UserRole, data);

  // Add to tree
  this->UI.descriptors->addTopLevelItem(item);
  this->UI.descriptors->setCurrentItem(item);

  // Mark as modified
  this->descriptorsChanged();

  // Edit the name
  this->UI.descriptors->editItem(item);
}

//-----------------------------------------------------------------------------
void vqConfigureDialog::removeDescriptors()
{
  QTreeWidget* tree = this->UI.descriptors;
  QList<QTreeWidgetItem*> selection = tree->selectedItems();

  foreach (QTreeWidgetItem* item, selection)
    delete tree->takeTopLevelItem(tree->indexOfTopLevelItem(item));

  this->descriptorsChanged();
}
