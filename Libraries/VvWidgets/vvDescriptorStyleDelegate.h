// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
