/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvQueryServerDialog.h"
#include "ui_vvQueryServerDialog.h"

#include <QPushButton>

#include <qtScopedValueChange.h>
#include <qtUtil.h>

#include "vvCustomQueryServerChooser.h"

QTE_IMPLEMENT_D_FUNC(vvQueryServerDialog)

namespace // anonymous
{

struct ChooserInfo
{
  QString displayName;
  QRegExp schemes;
  vvAbstractQueryServerChooser* chooser;
};

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vvQueryServerDialogPrivate
{
public:
  Ui::vvQueryServerDialog UI;
  QList<ChooserInfo> choosers;
  QUrl uri;
};

//-----------------------------------------------------------------------------
vvQueryServerDialog::vvQueryServerDialog(QWidget* parent, Qt::WindowFlags f) :
  QDialog(parent, f),
  d_ptr(new vvQueryServerDialogPrivate)
{
  QTE_D(vvQueryServerDialog);
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Disable accepting with no valid URI
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  connect(d->UI.protocol, SIGNAL(currentIndexChanged(int)),
          this, SLOT(serverTypeChanged(int)));
}

//-----------------------------------------------------------------------------
vvQueryServerDialog::~vvQueryServerDialog()
{
}

//-----------------------------------------------------------------------------
QUrl vvQueryServerDialog::uri() const
{
  QTE_D_CONST(vvQueryServerDialog);
  return d->uri;
}

//-----------------------------------------------------------------------------
void vvQueryServerDialog::registerServerType(
  const QString& displayName,
  const QRegExp& supportedSchemes,
  vvAbstractQueryServerChooser* chooser)
{
  QTE_D(vvQueryServerDialog);

  ChooserInfo ci;
  ci.displayName = displayName;
  ci.schemes = supportedSchemes;
  ci.chooser = chooser;
  d->choosers.append(ci);

  qtScopedBlockSignals b(d->UI.protocol);
  d->UI.protocol->addItem(displayName);
  d->UI.configurationStack->addWidget(chooser);

  connect(chooser, SIGNAL(uriChanged(QUrl)),
          this, SLOT(setUri(QUrl)));
}

//-----------------------------------------------------------------------------
int vvQueryServerDialog::exec(const QUrl& initialUri)
{
  QTE_D(vvQueryServerDialog);

  // Register 'custom' chooser
  if (d->choosers.isEmpty() || d->choosers.last().schemes != QRegExp(".*"))
    {
    this->registerServerType("(custom)", QRegExp(".*"),
                             new vvCustomQueryServerChooser);
    }

  // Select chooser based on initial URI
  int k = d->choosers.count();
  while (k--)
    {
    const ChooserInfo& ci = d->choosers[k];
    ci.chooser->setUri(initialUri);
    if (ci.schemes.exactMatch(initialUri.scheme()))
      {
      qtScopedBlockSignals b(d->UI.protocol);
      d->UI.protocol->setCurrentIndex(k);
      }
    }
  this->serverTypeChanged(d->UI.protocol->currentIndex());

  return QDialog::exec();
}

//-----------------------------------------------------------------------------
void vvQueryServerDialog::serverTypeChanged(int newIndex)
{
  QTE_D(vvQueryServerDialog);

  if (newIndex < 0 || newIndex > d->choosers.count())
    {
    return;
    }

  d->UI.configurationStack->setCurrentIndex(newIndex);
  this->setUri(d->choosers[newIndex].chooser->uri());
}

//-----------------------------------------------------------------------------
void vvQueryServerDialog::setUri(QUrl newUri)
{
  QTE_D(vvQueryServerDialog);
  d->uri = newUri;

  // Allow accepting if(f) URI is valid
  QPushButton* okayButton = d->UI.buttonBox->button(QDialogButtonBox::Ok);
  okayButton->setEnabled(newUri.isValid() && !newUri.isRelative());
}
