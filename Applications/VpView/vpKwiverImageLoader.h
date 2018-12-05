/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpKwiverImageLoader_h
#define __vpKwiverImageLoader_h

#include <vital/types/image_container.h>

#include <qtGlobal.h>

#include <QScopedPointer>

#include <string>

class vpKwiverImageLoaderPrivate;

class vpKwiverImageLoader
{
public:
  vpKwiverImageLoader();
  ~vpKwiverImageLoader();

  kwiver::vital::image_container_sptr load(const std::string& filename);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpKwiverImageLoader)

private:
  QTE_DECLARE_PRIVATE(vpKwiverImageLoader)
  QTE_DISABLE_COPY(vpKwiverImageLoader)
};

#endif
