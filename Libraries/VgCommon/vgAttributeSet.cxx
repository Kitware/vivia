/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgAttributeSet.h"

//-----------------------------------------------------------------------------
void vgAttributeSet::SetMask(const std::string& group,
                             const std::string& name,
                             unsigned int mask)
{
  std::vector<vgAttribute>& attrs = this->AttributeGroups[group].Attributes;
  for (size_t i = 0, size = attrs.size(); i < size; ++i)
    {
    if (attrs[i].Name == name)
      {
      attrs[i].Mask = mask;
      return;
      }
    }

  // attribute doesn't exist yet, so push it into the group
  vgAttribute attr;
  attr.Name = name;
  attr.Mask = mask;
  attrs.push_back(attr);
}

//-----------------------------------------------------------------------------
unsigned int vgAttributeSet::GetMask(const std::string& group,
                                     const std::string& name) const
{
  AttributeGroupMap::const_iterator iter = this->AttributeGroups.find(group);
  if (iter == this->AttributeGroups.end())
    {
    return 0;
    }

  for (size_t i = 0, size = iter->second.Attributes.size(); i < size; ++i)
    {
    if (iter->second.Attributes[i].Name == name)
      {
      return iter->second.Attributes[i].Mask;
      }
    }

  // not found
  return 0;
}

//-----------------------------------------------------------------------------
unsigned int vgAttributeSet::GetGroupMask(const std::string& group) const
{
  AttributeGroupMap::const_iterator iter = this->AttributeGroups.find(group);
  if (iter == this->AttributeGroups.end())
    {
    return 0;
    }

  unsigned int groupMask = 0;
  for (size_t i = 0, size = iter->second.Attributes.size(); i < size; ++i)
    {
    groupMask |= iter->second.Attributes[i].Mask;
    }

  return groupMask;
}

//-----------------------------------------------------------------------------
void vgAttributeSet::SetGroupEnabled(const std::string& group, bool enabled)
{
  this->AttributeGroups[group].Enabled = enabled;
}

//-----------------------------------------------------------------------------
bool vgAttributeSet::IsGroupEnabled(const std::string& group) const
{
  AttributeGroupMap::const_iterator iter = this->AttributeGroups.find(group);
  if (iter == this->AttributeGroups.end())
    {
    return false;
    }

  return iter->second.Enabled;
}

//-----------------------------------------------------------------------------
std::vector<std::string> vgAttributeSet::GetGroups() const
{
  std::vector<std::string> groups;
  for (AttributeGroupMap::const_iterator iter = this->AttributeGroups.begin(),
       end = this->AttributeGroups.end(); iter != end; ++iter)
    {
    groups.push_back(iter->first);
    }

  return groups;
}

//-----------------------------------------------------------------------------
std::vector<std::string> vgAttributeSet::GetEnabledGroups() const
{
  std::vector<std::string> groups;
  for (AttributeGroupMap::const_iterator iter = this->AttributeGroups.begin(),
       end = this->AttributeGroups.end(); iter != end; ++iter)
    {
    if (iter->second.Enabled)
      {
      groups.push_back(iter->first);
      }
    }

  return groups;
}

//-----------------------------------------------------------------------------
std::vector<vgAttribute> vgAttributeSet::GetAttributes(const std::string& group) const
{
  AttributeGroupMap::const_iterator iter = this->AttributeGroups.find(group);
  if (iter == this->AttributeGroups.end())
    {
    return std::vector<vgAttribute>();
    }

  return iter->second.Attributes;
}

//-----------------------------------------------------------------------------
void vgAttributeSet::Clear()
{
  this->AttributeGroups.clear();
}
