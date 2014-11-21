/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vgExport.h>

#include <qtFlagListDelegate.h>

#include "vvDescriptorStyle.h"

// The instantiation must be in a separate translation unit for MSVC, which for
// some reason does not allow a template instantiation to follow a template
// declaration (i.e. an 'extern template' of the same specialization).

template class VV_WIDGETS_EXPORT
qtFlagListDelegate <vvDescriptorStyle::StyleFlag, vvDescriptorStyle::Styles>;
