// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgfNamespace_h
#define __vgfNamespace_h

#include <vgExport.h>

#include <QMetaType>
#include <Qt>

class QIcon;

/// \namespace vgf Global definitions for the VisGUI GUI Framework
namespace vgf
{
  /// Well known item types.
  enum ItemType
    {
    InvalidItemType = 0,
    TrackItem       = 0x01,
    EventItem       = 0x02,
    ActivityItem    = 0x04,
    QueryResultItem = 0x08,
    RegionItem      = 0x10,
    AnnotationItem  = 0x20,
    BuiltinItems    = 0x2f
    };

  /// Set of well known item types.
  Q_DECLARE_FLAGS(ItemTypes, ItemType)

  /// Common data roles for Qt item models.
  ///
  /// This enumeration defines a number of standard data roles used by
  /// framework item views. Item models used to feed framework items views are
  /// expected to provide a subset of these data roles as determined by the
  /// specific view being used.
  enum ItemDataRole
    {
    /// (QString) Name or ID of the item as it should be displayed to the user.
    NameRole = Qt::DisplayRole,

    /// (vgf::ItemType) Type of the item.
    ItemTypeRole = Qt::UserRole,

    /// (\em varies) Scene ID of the item.
    ///
    /// This typically is a vtkIdType that uniquely identifies the item with a
    /// corresponding data node (especially in the case of aggregator nodes)
    /// and is useful for synchronization with other views, but has no meaning
    /// to the user.
    IdentityRole,
    /// (vgfItemReference) Scene item reference of the item.
    InternalReferenceRole,
    /// (\em varies) Logical ID of the item.
    ///
    /// The data type depends on the item type. Usually this will match or
    /// contain the ID that the item was given by the source that produced the
    /// item, and may contain a reference to the item's source.
    LogicalIdentityRole,
    /// (QUuid) Universally unique identifier of the item.
    UniqueIdentityRole,

    /// (bool) Effective visibility state of the item.
    ///
    /// This is \c true iff the item is visible (or would be visible, when the
    /// scene time overlaps the item's temporal extents) in the scene. It is
    /// \c false if the item is filtered, has been hidden by the user, etc.
    VisibilityRole,
    /// (bool) User override visibility state of the item.
    ///
    /// This is \c false iff the user has manually specified that this specific
    /// item should be hidden.
    UserVisibilityRole,
    /// (vgTimeStamp) Scene time at which the item enters scope.
    StartTimeRole,
    /// (vgTimeStamp) Scene time at which the item leaves scope.
    EndTimeRole,

    /// (TODO) User adjudication score assigned to the item.
    RatingRole,
    /// (bool) 'Starred' state of the item.
    StarRole,
    /// (QString) User note associated with the item.
    NoteRole,

    /// (int) Best currently applicable computed classification of the item.
    ///
    /// The value usually corresponds to a value in a classification type
    /// registry for the item type.
    ClassificationRole,
    /// (double) Computed confidence score for the item.
    ///
    /// Depending on the item type, this might be the relevancy score of a
    /// query result, probability of an event detection, confidence of a track
    /// classification, etc.
    ConfidenceRole,
    // TODO FullClassificationRole

    /// First role that can be used for model-specific purposes.
    UserRole = Qt::UserRole + 224
    };

  /// Desired behavior flags for a 'jump to' request.
  ///
  /// When executing a 'jump to' for a target item, these flags describe how
  /// the view should be spatially and/or temporally altered to fulfill the
  /// request.
  enum JumpFlag
    {
    /// Jump should go to the start time of the target.
    JumpToStartTime     = 0x01,
    /// Jump should go to the end time of the target.
    JumpToEndTime       = 0x02,
    /// Jump should center the view on the target's position.
    JumpToPosition      = 0x10,
    /// Jump should zoom the view onto the target.
    JumpToScale         = 0x20,
    /// Bitmask of all temporal locality flags
    JumpTemporalMask    = 0x0f,
    /// Bitmask of all spatial locality flags
    JumpSpatialMask     = 0xf0,
    };

  /// Desired behavior for a 'jump to' request.
  Q_DECLARE_FLAGS(JumpFlags, JumpFlag)

  /// Get icon for specified item type.
  ///
  /// This returns the standard icon used to represent the specified item type
  /// (e.g. in a tree view). If no icon is available for the specified type,
  /// the returned icon will be empty.
  extern VG_GUI_FRAMEWORK_EXPORT QIcon itemTypeIcon(ItemType, int size);
}

Q_DECLARE_METATYPE(vgf::ItemType)
Q_DECLARE_METATYPE(vgf::JumpFlags)

Q_DECLARE_OPERATORS_FOR_FLAGS(vgf::ItemTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(vgf::JumpFlags)

#endif
