/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvDescriptorStyleDelegate.h"

namespace
{

static const vvDescriptorStyle::StyleFlag styles[] =
{
  vvDescriptorStyle::Articulation,
  vvDescriptorStyle::Trajectory,
  vvDescriptorStyle::Appearance,
  vvDescriptorStyle::Color,
  vvDescriptorStyle::Metadata,
  vvDescriptorStyle::None
};

}

//-----------------------------------------------------------------------------
vvDescriptorStyleDelegate::vvDescriptorStyleDelegate(QObject* parent) :
  qtFlagListDelegate <vvDescriptorStyle::StyleFlag,
                      vvDescriptorStyle::Styles> (parent)
{
  QStringList names;
  QVariantList values;
  for (int i = 0; styles[i] != vvDescriptorStyle::None; ++i)
    {
    QVariant value =
      QVariant::fromValue<vvDescriptorStyle::StyleFlag>(styles[i]);
    names.append(vvDescriptorStyle::string(styles[i]));
    values.append(value);
    }
  this->setMapping(names, values);
}
