/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QFileInfo>
#include <QMetaType>
#include <QUrl>

#include <qtOnce.h>

#include <vgFileDialog.h>

#include "vvQuerySession.h"

namespace // anonymous
{

QTE_ONCE(rmtGuard);

//-----------------------------------------------------------------------------
void registerMetaTypes()
{
  QTE_REGISTER_METATYPE(qtStatusSource);
  QTE_REGISTER_METATYPE(vvProcessingRequest);
  QTE_REGISTER_METATYPE(vvQueryInstance);
  QTE_REGISTER_METATYPE(vvQueryResult);
  QTE_REGISTER_METATYPE(vvIqr::ScoringClassifiers);
  QTE_REGISTER_METATYPE(QList<vvTrack>);
  QTE_REGISTER_METATYPE(QList<vvDescriptor>);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vvQuerySession::vvQuerySession()
{
  qtOnce(rmtGuard, &registerMetaTypes);
}

//-----------------------------------------------------------------------------
vvQuerySession::~vvQuerySession()
{
}

//-----------------------------------------------------------------------------
QUrl vvQuerySession::getFormulationSourceUri(
  const QUrl& oldUri, vvQueryFormulationType type, QWidget* dialogParent)
{
  QFileInfo vfi(oldUri.toLocalFile());
  QString fileName =
    vgFileDialog::getOpenFileName(
      dialogParent, "Select video or image from which to formulate query...",
      vfi.dir().path(),
      "Common Formats (*.jpg *.png *.mpg);;"
      "MPEG Videos (*.mpg);;"
      "Common Image Formats (*.jpg *.png);;"
      "All files (*)");

  QUrl uri;
  if (!fileName.isEmpty())
    {
    uri = QUrl::fromLocalFile(fileName);
    uri = this->fixupFormulationSourceUri(uri, type);
    }
  return uri;
}

//-----------------------------------------------------------------------------
QUrl vvQuerySession::fixupFormulationSourceUri(
  QUrl uri, vvQueryFormulationType type)
{
  if (!uri.isEmpty())
    {
    uri.setEncodedQuery(QByteArray());
    switch (type)
      {
      case vvQueryFormulation::FromImage:
        uri.addQueryItem("FormulationType", "Image");
        break;
      case vvQueryFormulation::FromVideo:
        uri.addQueryItem("FormulationType", "Video");
        break;
      default:
        break;
      }
    }
  return uri;
}
