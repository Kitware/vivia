// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsFactoryAction_h
#define __vsFactoryAction_h

#include <QString>
#include <QUrl>

struct vsPendingFactoryAction
{
  QString FactoryIdentifier;
  QUrl SourceUri;
};

#endif
