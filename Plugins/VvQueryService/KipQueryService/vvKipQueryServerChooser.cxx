/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvKipQueryServerChooser.h"
#include "ui_vvKipQueryServerChooser.h"

#include <vgFileDialog.h>

QTE_IMPLEMENT_D_FUNC(vvKipQueryServerChooser)

//-----------------------------------------------------------------------------
class vvKipQueryServerChooserPrivate
{
public:
  Ui::vvKipQueryServerChooser UI;
  QUrl uri;
};

//-----------------------------------------------------------------------------
vvKipQueryServerChooser::vvKipQueryServerChooser(QWidget* parent) :
  vvAbstractQueryServerChooser(parent),
  d_ptr(new vvKipQueryServerChooserPrivate)
{
  QTE_D(vvKipQueryServerChooser);
  d->UI.setupUi(this);

  connect(d->UI.pipeline, SIGNAL(textChanged(QString)),
          this, SLOT(updateUri()));
  connect(d->UI.pipelineBrowse, SIGNAL(clicked()),
          this, SLOT(browseForPipeline()));
}

//-----------------------------------------------------------------------------
vvKipQueryServerChooser::~vvKipQueryServerChooser()
{
}

//-----------------------------------------------------------------------------
QUrl vvKipQueryServerChooser::uri() const
{
  QTE_D_CONST(vvKipQueryServerChooser);
  return d->uri;
}

//-----------------------------------------------------------------------------
void vvKipQueryServerChooser::setUri(QUrl newUri)
{
  QTE_D(vvKipQueryServerChooser);

  d->UI.pipeline->setText(newUri.queryItemValue("Pipeline"));
  this->updateUri();
}

//-----------------------------------------------------------------------------
void vvKipQueryServerChooser::updateUri()
{
  QTE_D(vvKipQueryServerChooser);

  d->uri = QUrl("kip:");
  d->uri.addQueryItem("Pipeline", d->UI.pipeline->text());

  emit this->uriChanged(d->uri);
}

//-----------------------------------------------------------------------------
void vvKipQueryServerChooser::browseForPipeline()
{
  QTE_D(vvKipQueryServerChooser);

  QString newPath = vgFileDialog::getOpenFileName(
                      this, "Pipeline File...", d->UI.pipeline->text(),
                      "Pipeline files (*.pipe);;All files (*)");
  if (!newPath.isEmpty())
  {
    d->UI.pipeline->setText(newPath);
  }
}
