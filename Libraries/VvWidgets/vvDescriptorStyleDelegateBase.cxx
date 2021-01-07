// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <vgExport.h>

#include <qtFlagListDelegate.h>

#include "vvDescriptorStyle.h"

// The instantiation must be in a separate translation unit for MSVC, which for
// some reason does not allow a template instantiation to follow a template
// declaration (i.e. an 'extern template' of the same specialization).

template class VV_WIDGETS_EXPORT
qtFlagListDelegate <vvDescriptorStyle::StyleFlag, vvDescriptorStyle::Styles>;
