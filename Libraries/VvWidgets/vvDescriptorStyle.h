// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvDescriptorStyle_h
#define __vvDescriptorStyle_h

#include <QMetaType>
#include <QVariantHash>

#include <vgExport.h>

struct vvDescriptor;

//-----------------------------------------------------------------------------
namespace vvDescriptorStyle
{
  enum StyleFlag
    {
    None          = 0,
    Articulation  = 0x01,
    Trajectory    = 0x02,
    Appearance    = 0x04,
    Color         = 0x08,
    Metadata      = 0x80,
    All           = 0xff
    };
  Q_DECLARE_FLAGS(Styles, StyleFlag);

  enum ScopeItem
    {
    Process       = 0x1,
    Settings      = 0x2,
    Any           = 0xf
    };
  Q_DECLARE_FLAGS(Scope, ScopeItem)

  class Map;

  VV_WIDGETS_EXPORT QString string(StyleFlag);
  VV_WIDGETS_EXPORT QString string(Styles);

  VV_WIDGETS_EXPORT Styles styles(const vvDescriptor&, bool* known = 0);
  VV_WIDGETS_EXPORT Styles styles(const vvDescriptor&, const Map&,
                                  bool* known = 0);
  VV_WIDGETS_EXPORT QString styleString(const vvDescriptor&);
  VV_WIDGETS_EXPORT QString styleString(const vvDescriptor&, const Map&);

  // Get the global descriptor style mappings
  VV_WIDGETS_EXPORT Map map(Scope scope = Any);

  // Set the global descriptor style mappings
  VV_WIDGETS_EXPORT void setMap(const Map&, ScopeItem where = Process);
}

//-----------------------------------------------------------------------------
class VV_WIDGETS_EXPORT vvDescriptorStyle::Map
  : public QHash<QString, vvDescriptorStyle::Styles>
{
public:
  Map();
  Map(const Map&);
  Map(QHash<QString, Styles> const&);
  Map(const QVariantHash&);

  Map& operator=(const Map& other);
  Map& operator=(QHash<QString, Styles> const& other);
  Map& operator=(const QVariantHash& other);

  QVariantHash serializable() const;
};

//-----------------------------------------------------------------------------
Q_DECLARE_OPERATORS_FOR_FLAGS(vvDescriptorStyle::Styles)
Q_DECLARE_OPERATORS_FOR_FLAGS(vvDescriptorStyle::Scope)

Q_DECLARE_METATYPE(vvDescriptorStyle::StyleFlag)
Q_DECLARE_METATYPE(vvDescriptorStyle::Styles)

#endif
