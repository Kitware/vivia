// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTypeDefs_h
#define __vtkVgTypeDefs_h

struct vgObjectTypeDefinitions
{
  enum enumObjectTypes
    {
    Activity,
    Event,
    Track,
    SceneElement
    };
};

struct vgObjectStatus
{
  enum enumObjectStatus
    {
    None,
    Adjudicated,
    Excluded,
    Positive = Adjudicated,
    Negative = Excluded
    };
};

struct vgEventTypeDefinitions
{
  enum enumEventCategories
    {
    Event = 0,
    Activity,
    Intel,
    CategoryUnknown
    };
};

struct vgSpecialIconTypeDefinitions
{
  enum enumSpecialIcons
    {
    Intelligence = 4,
    Follower = 5,
    Unknown = 100
    };
};

#endif // __vtkVgTypeDefs_h
