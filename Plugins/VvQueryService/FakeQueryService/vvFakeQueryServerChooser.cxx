/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvFakeQueryServerChooser.h"
#include "ui_vvFakeQueryServerChooser.h"

#include <vgFileDialog.h>

#include <QUrlQuery>

QTE_IMPLEMENT_D_FUNC(vvFakeQueryServerChooser)

//-----------------------------------------------------------------------------
class vvFakeQueryServerChooserPrivate
{
public:
  Ui::vvFakeQueryServerChooser UI;
  QUrl uri;
};

//-----------------------------------------------------------------------------
vvFakeQueryServerChooser::vvFakeQueryServerChooser(QWidget* parent) :
  vvAbstractQueryServerChooser(parent),
  d_ptr(new vvFakeQueryServerChooserPrivate)
{
  QTE_D(vvFakeQueryServerChooser);
  d->UI.setupUi(this);

  connect(d->UI.archive, SIGNAL(textChanged(QString)),
          this, SLOT(updateUri()));
  connect(d->UI.archiveBrowse, SIGNAL(clicked()),
          this, SLOT(browseForArchive()));
}

//-----------------------------------------------------------------------------
vvFakeQueryServerChooser::~vvFakeQueryServerChooser()
{
}

//-----------------------------------------------------------------------------
QUrl vvFakeQueryServerChooser::uri() const
{
  QTE_D_CONST(vvFakeQueryServerChooser);
  return d->uri;
}

//-----------------------------------------------------------------------------
void vvFakeQueryServerChooser::setUri(QUrl newUri)
{
  QTE_D(vvFakeQueryServerChooser);

  d->UI.archive->setText(QUrlQuery{newUri}.queryItemValue("Archive"));
  this->updateUri();
}

//-----------------------------------------------------------------------------
void vvFakeQueryServerChooser::updateUri()
{
  QTE_D(vvFakeQueryServerChooser);

  auto query = QUrlQuery{};
  query.addQueryItem("Archive", d->UI.archive->text());

  d->uri = QUrl("fake:");
  d->uri.setQuery(query);

  emit this->uriChanged(d->uri);
}

//-----------------------------------------------------------------------------
void vvFakeQueryServerChooser::browseForArchive()
{
  QTE_D(vvFakeQueryServerChooser);

  QString newPath = vgFileDialog::getExistingDirectory(
                      this, "Archive Directory...", d->UI.archive->text());
  if (!newPath.isEmpty())
    {
    d->UI.archive->setText(newPath);
    }
}
