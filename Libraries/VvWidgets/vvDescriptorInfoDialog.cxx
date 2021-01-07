// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvDescriptorInfoDialog.h"
#include "ui_vvDescriptorInfoDialog.h"

#include <qtStlUtil.h>
#include <qtUtil.h>

#include <vgCheckArg.h>

QTE_IMPLEMENT_D_FUNC(vvDescriptorInfoDialog)

//-----------------------------------------------------------------------------
class vvDescriptorInfoDialogPrivate
{
public:
  Ui::vvDescriptorInfoDialog UI;
  QHash<qint64, vvDescriptor> descriptors;
};

//-----------------------------------------------------------------------------
vvDescriptorInfoDialog::vvDescriptorInfoDialog(
  QWidget* parent, Qt::WindowFlags f) :
  QDialog(parent, f),
  d_ptr(new vvDescriptorInfoDialogPrivate)
{
  QTE_D(vvDescriptorInfoDialog);
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  connect(d->UI.toggleDetails, SIGNAL(toggled(bool)),
          this, SLOT(toggleDetails(bool)));
  connect(d->UI.descriptorTree,
          SIGNAL(descriptorSelectionChanged(QList<vvDescriptor>)),
          this, SLOT(showDetails(QList<vvDescriptor>)));

  d->UI.detailsPane->setVisible(d->UI.toggleDetails->isChecked());
  this->showDetails(QList<vvDescriptor>());
}

//-----------------------------------------------------------------------------
vvDescriptorInfoDialog::~vvDescriptorInfoDialog()
{
}

//-----------------------------------------------------------------------------
QList<vvDescriptor> vvDescriptorInfoDialog::descriptors() const
{
  QTE_D_CONST(vvDescriptorInfoDialog);
  return d->descriptors.values();
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoDialog::setDescriptors(
  QList<vvDescriptor> newDescriptors)
{
  QTE_D(vvDescriptorInfoDialog);

  d->descriptors.clear();

  QList<qint64> ids;
  qint64 n = 0;
  for (int i = 0, k = newDescriptors.count(); i < k; ++i)
    {
    d->descriptors.insert(n, newDescriptors[i]);
    ids.append(n);
    ++n;
    }

  d->UI.descriptorTree->clear();
  d->UI.descriptorTree->setDescriptors(d->descriptors);
  d->UI.descriptorTree->addDescriptorItems(ids);
  qtUtil::resizeColumnsToContents(d->UI.descriptorTree);
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoDialog::setDescriptors(std::vector<vvDescriptor> v)
{
  this->setDescriptors(qtList(v));
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoDialog::clearDescriptors()
{
  this->setDescriptors(QList<vvDescriptor>());
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoDialog::showDetails(QList<vvDescriptor> descriptors)
{
  QTE_D(vvDescriptorInfoDialog);

  if (descriptors.isEmpty())
    {
    d->UI.detailsPane->setCurrentWidget(d->UI.detailsMessage);
    d->UI.detailsSummary->setText("No descriptor selected");
    }
  else if (descriptors.count() > 1)
    {
    d->UI.detailsPane->setCurrentWidget(d->UI.detailsMessage);
    d->UI.detailsSummary->setText("Multiple descriptors selected");
    }
  else
    {
    d->UI.detailsPane->setCurrentWidget(d->UI.descriptorDetails);
    d->UI.descriptorDetails->setDescriptor(descriptors.first());
    }
}

//-----------------------------------------------------------------------------
void vvDescriptorInfoDialog::toggleDetails(bool detailsVisible)
{
  QTE_D(vvDescriptorInfoDialog);
  CHECK_ARG(d->UI.detailsPane->isVisible() != detailsVisible);

  QSize newSize = this->size();
  QSize detailsSize = d->UI.descriptorDetails->size();
  if (detailsVisible)
    {
    d->UI.detailsPane->setVisible(true);
    newSize.setHeight(newSize.height() + detailsSize.height());
    }
  else
    {
    newSize.setHeight(newSize.height() - detailsSize.height());
    d->UI.detailsPane->setVisible(false);
    }
  this->resize(newSize);
}
