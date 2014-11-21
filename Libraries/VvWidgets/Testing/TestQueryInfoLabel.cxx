/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QApplication>
#include <QFileInfo>
#include <QUrl>

#include <vvHeader.h>
#include <vvReader.h>

#include "../vvQueryInfoLabel.h"

//-----------------------------------------------------------------------------
QString loadQuery(QString fileName, vvQueryInfoLabel& label)
{
  if (fileName.isEmpty())
    {
    return "No query file specified";
    }

  if (!QFileInfo(fileName).exists())
    {
    return "File not found";
    }

  vvReader reader;
  if (!reader.open(QUrl::fromLocalFile(fileName)))
    {
    return reader.error();
    }

  vvHeader header;
  if (!reader.readHeader(header))
    {
    return reader.error();
    }

  if (header.type != vvHeader::QueryPlan)
    {
    return "File does not contain a query plan";
    }

  vvQueryInstance query;
  if (!reader.readQueryPlan(query))
    {
    return reader.error();
    }

  if (query.isSimilarityQuery())
    {
    label.setDescriptors(query.constSimilarityQuery()->Descriptors);
    return "No descriptors";
    }
  return "File is a retrieval query";
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  // Set organization, so we can find the user's descriptor style map
  app.setOrganizationName("Kitware");

  vvQueryInfoLabel label;

  QString error = loadQuery(app.arguments().value(1), label);
  label.setErrorText(QString("(%1)").arg(error));

  label.show();
  return app.exec();
}
