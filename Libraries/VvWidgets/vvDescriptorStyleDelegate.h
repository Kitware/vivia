/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvDescriptorStyleDelegate_h
#define __vvDescriptorStyleDelegate_h

#include <qtFlagListDelegate.h>

#include <vgExport.h>

#include "vvDescriptorStyle.h"

VV_WIDGETS_EXPORT_TEMPLATE template class VV_WIDGETS_EXPORT
qtFlagListDelegate <vvDescriptorStyle::StyleFlag, vvDescriptorStyle::Styles>;

class VV_WIDGETS_EXPORT vvDescriptorStyleDelegate :
  public qtFlagListDelegate <vvDescriptorStyle::StyleFlag,
                             vvDescriptorStyle::Styles>
{
  Q_OBJECT

public:
  vvDescriptorStyleDelegate(QObject* parent = 0);
};

#endif
