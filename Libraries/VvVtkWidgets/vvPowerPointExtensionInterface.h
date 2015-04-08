/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvPowerPointExtensionInterface_h
#define __vvPowerPointExtensionInterface_h

#include <vtkType.h>

#include <QHash>
#include <QList>

class vtkVgTrack;
class vtkVgEvent;

class vvPowerPointSlideGenerator;

//-----------------------------------------------------------------------------
/// Input data for PowerPoint extensions.
struct vvPowerPointInput
{
  /// List of available tracks
  QList<vtkVgTrack*> tracks;

  /// List of available events
  QList<vtkVgEvent*> events;

  // TODO add other information here (e.g. type registries) as needed
};

//-----------------------------------------------------------------------------
/// Collection of slide generator items for creating a PowerPoint report.
struct vvPowerPointItemCollection
{
  /// List of items that will generate slides in the output presentation.
  ///
  /// This provides a list of generator classes that will be used to create
  /// slides in the output presentation. The generators will be invoked in the
  /// order in which they appear in this list.
  ///
  /// Extensions may add items to this list and/or remove items from the list.
  /// Removed items should be deleted using <code>operator delete</code>.
  QList<vvPowerPointSlideGenerator*> items;

  /// Map of items based on their underlying entity.
  ///
  /// This provides a mapping of the items in the above list from their
  /// associated entity, e.g. a \c vtkVgTrack*, \c vtkVgEvent*, etc. Extensions
  /// that wish to remove an item for a specific entity may use this map to
  /// find the item.
  QHash<void*, vvPowerPointSlideGenerator*> itemMap;
};

//-----------------------------------------------------------------------------
/// Interface for a PowerPoint extension.
///
/// This class defines the public interface for a PowerPoint extension. A
/// PowerPoint extension provides a mechanism for plugins to add or modify
/// content when generating a PowerPoint report.
class vvPowerPointExtensionInterface
{
public:
  virtual ~vvPowerPointExtensionInterface() {}

  /// Add or update items in the slide generator item collection.
  ///
  /// This method is called for each registered plugin when generating a
  /// PowerPoint report in order to allow the extension to make changes to the
  /// collection of slide generator \p items. An extension may add, remove, or
  /// rearrange items in order to affect the content of the generated
  /// presentation. When adding items, the extension must allocate the items
  /// such that they can be destroyed using <code>operator delete</code>, and,
  /// where possible, should update both vvPowerPointItemCollection::items and
  /// vvPowerPointItemCollection::itemMap.
  virtual void updateItems(const vvPowerPointInput& input,
                           vvPowerPointItemCollection& items) = 0;
};

Q_DECLARE_INTERFACE(vvPowerPointExtensionInterface,
                    "org.visgui.vvPowerPointExtensionInterface")

#endif
