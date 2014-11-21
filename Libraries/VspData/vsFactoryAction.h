/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
