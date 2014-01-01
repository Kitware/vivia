/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgAboutDialog.h"
#include "vgApplication.h"

#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <QPushButton>

#include <qtUtil.h>

#include <vgFingerprint.h>

#include "ui_vgAboutDialog.h"

QTE_IMPLEMENT_D_FUNC(vgAboutDialog)

namespace // anonymous
{

const char* clipTextTemplate =
  "Application Name: @APP_TITLE@\n"
  "Application Version: @APP_VERSION@\n"
  "Qt Version: @QT_VERSION@\n"
  "Copyright: @COPY_YEAR@ @APP_ORGANIZATION@\n"
  "Build Identifier: @BUILD@\n";

//-----------------------------------------------------------------------------
QString genBuildDisplayString()
{
  const QString& str = QString::fromLocal8Bit(GetVisguiSha1()).left(12);
  const QString& status = QString::fromLocal8Bit(GetVisguiStatus());
  return (status.isEmpty() ? str : str + " *");
}

//-----------------------------------------------------------------------------
QString genBuildCopyString()
{
  const QString& hash = QString::fromLocal8Bit(GetVisguiSha1());
  const QString& status = QString::fromLocal8Bit(GetVisguiStatus());
  return QString("%2 (%1)").arg(status.isEmpty() ? "clean" : "dirty")
                           .arg(hash);
}

//-----------------------------------------------------------------------------
QString genText(QString formatStr, const QString& buildIdentifier)
{
  formatStr.replace("@APP_TITLE@", vgApplication::applicationName());
  formatStr.replace("@APP_VERSION@", vgApplication::applicationVersion());
  formatStr.replace("@QT_VERSION@", QString::fromLocal8Bit(qVersion()));
  formatStr.replace("@COPY_YEAR@", vgApplication::copyrightYear());
  formatStr.replace("@APP_ORGANIZATION@", vgApplication::copyrightOrganization());
  formatStr.replace("@BUILD@", buildIdentifier);
  return formatStr;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vgAboutDialogPrivate
{
public:
  Ui::vgAboutDialog UI;
  QString copyText;
  QMenu* contextMenu;
};

//-----------------------------------------------------------------------------
vgAboutDialog::vgAboutDialog(QWidget* parent, Qt::WindowFlags f) :
  QDialog(parent, f),
  d_ptr(new vgAboutDialogPrivate)
{
  QTE_D(vgAboutDialog);

  // Set up UI
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);
  this->setWindowTitle(QString("About %1").arg(qApp->applicationName()));

  // Replace placeholders in templates with real information
  d->UI.label->setText(genText(d->UI.label->text(), genBuildDisplayString()));
  d->copyText = genText(clipTextTemplate, genBuildCopyString());

  // Load application about icon
  QIcon icon = qApp->windowIcon();
  if (icon.isNull())
    {
    d->UI.image->hide();
    }
  else
    {
    QPixmap pixmap = icon.pixmap(128, 128);
    d->UI.image->setPixmap(pixmap);
    d->UI.image->setFixedSize(pixmap.size());
    }

  // Set up context menu
  d->UI.label->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(d->UI.label, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(showContextMenu(QPoint)));

  d->contextMenu = new QMenu(this);
  QAction* copyAction = new QAction("&Copy to clipboard", this);
  connect(copyAction, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
  d->contextMenu->addAction(copyAction);
}

//-----------------------------------------------------------------------------
vgAboutDialog::~vgAboutDialog()
{
}

//-----------------------------------------------------------------------------
int vgAboutDialog::exec()
{
  this->resize(this->minimumSize());
  return QDialog::exec();
}

//-----------------------------------------------------------------------------
void vgAboutDialog::showContextMenu(QPoint pos)
{
  QTE_D(vgAboutDialog);
  d->contextMenu->exec(d->UI.label->mapToGlobal(pos));
}

//-----------------------------------------------------------------------------
void vgAboutDialog::copyToClipboard()
{
  QTE_D_CONST(vgAboutDialog);
  QApplication::clipboard()->setText(d->copyText);
}
