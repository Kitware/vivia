// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// Manages a set of grouped attributes (i.e. named bitmasks)

#ifndef __vgAttributeSet_h
#define __vgAttributeSet_h

#include <vgExport.h>

#include <map>
#include <string>
#include <vector>

struct vgAttribute
{
  vgAttribute() : Mask(0) {}

  std::string Name;
  unsigned long long Mask;
};

// Disable warning about the STL member of vgAttributeGroup not being exported.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

struct VG_COMMON_EXPORT vgAttributeGroup
{
  vgAttributeGroup() : Enabled(true) {}

  std::vector<vgAttribute> Attributes;
  bool Enabled;
};

class VG_COMMON_EXPORT vgAttributeSet
{
public:
  // Description:
  // Define or update an attribute mask
  void SetMask(const std::string& group, const std::string& name,
               unsigned long long mask);

  // Description:
  // Get the mask for a defined attribute
  unsigned long long GetMask(const std::string& group,
                             const std::string& name) const;

  // Description:
  // Get the OR'ed mask of all attributes currently defined for a group
  unsigned long long GetGroupMask(const std::string& group) const;

  // Description:
  // Set the enabled flag for a group. The group will be added if it didn't
  // already exist. Groups are enabled by default.
  void SetGroupEnabled(const std::string& group, bool enabled);

  // Description:
  // Get the enabled flag for a group
  bool IsGroupEnabled(const std::string& group) const;

  // Description:
  // Get all the defined groups. Expensive.
  std::vector<std::string> GetGroups() const;

  // Description:
  // Get all enabled groups. Expensive.
  std::vector<std::string> GetEnabledGroups() const;

  // Description:
  // Get all the attributes in a particular group
  std::vector<vgAttribute> GetAttributes(const std::string& group) const;

  // Description:
  // Reset to a clean state
  void Clear();

private:
  typedef std::map<std::string, vgAttributeGroup> AttributeGroupMap;

  AttributeGroupMap AttributeGroups;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __vgAttributeSet_h
