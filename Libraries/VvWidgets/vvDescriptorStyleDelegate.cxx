// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
