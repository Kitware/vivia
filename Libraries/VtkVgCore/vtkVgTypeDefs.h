/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
