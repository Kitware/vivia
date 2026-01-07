// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsFakeStreamFactory.h"

#include <QDebug>
#include <QFileInfo>
#include <QUrl>

#include <vgFileDialog.h>

#include "vsFakeStreamSource.h"

//-----------------------------------------------------------------------------
vsFakeStreamFactory::vsFakeStreamFactory()
{
}

//-----------------------------------------------------------------------------
vsFakeStreamFactory::~vsFakeStreamFactory()
{
}

//-----------------------------------------------------------------------------
bool vsFakeStreamFactory::initialize(QWidget* dialogParent)
{
  QString fileName = vgFileDialog::getOpenFileName(
                       dialogParent, "Load fake stream project...", QString(),
                       "vsPlay projects (*.prj);;"
                       "All files (*)");

  if (fileName.isEmpty())
    {
    return false;
    }

  return this->initialize(QUrl::fromLocalFile(fileName), dialogParent);
}

//-----------------------------------------------------------------------------
bool vsFakeStreamFactory::initialize(const QUrl& uri)
{
  return this->initialize(uri, 0);
}

//-----------------------------------------------------------------------------
bool vsFakeStreamFactory::initialize(const QUrl& uri, QWidget* dialogParent)
{
  if (uri.scheme() != "file")
    {
    const QString message =
     "Error creating stream from URI \"" + uri.toString() +
     "\": the scheme " + uri.scheme() + " is not supported.";
    this->warn(dialogParent, "Not supported", message);
    return false;
    }

  QFileInfo fi(uri.toLocalFile());
  if (!fi.exists())
    {
    return false;
    }
  QUrl canonicalUri = QUrl::fromLocalFile(fi.canonicalFilePath());

  this->setSource(new vsFakeStreamSource(canonicalUri));
  return true;
}
