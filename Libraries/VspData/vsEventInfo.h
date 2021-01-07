// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventInfo_h
#define __vsEventInfo_h

#include <vgColor.h>

#include <vgExport.h>

#include <QString>
#include <QList>

class QSettings;

class vgEventType;

struct vvEventSetInfo;

//-----------------------------------------------------------------------------
struct VSP_DATA_EXPORT vsEventInfo
{
  //.. Associated Enumerations and Data Types .................................

  /// Static event info template.
  ///
  /// This type provides a simplified version of vsEventInfo that is suitable
  /// for static initialization. The pen and label foreground colors may be
  /// specified; the label background color is always Qt::white.
  ///
  /// \sa events(QSettings&, const QString&, const vsEventInfo::Template*)
  struct Template
    {
    int type;               ///< Event type ID.
    const char* name;       ///< Event type name.
    unsigned char color[6]; ///< Pen and label foreground color.
    };

  enum Group
    {
    Unknown     = 0x0,
    Person      = 0x1,
    Vehicle     = 0x2,
    Classifier  = 0xf,
    Alert       = 0x10,
    General     = 0x20,
    User        = 0x80,
    NonUser     = 0x7f,
    All         = 0xff
    };
  Q_DECLARE_FLAGS(Groups, Group)

  enum Type
    {
    Tripwire        = -3000,
    EnteringRegion  = -3001,
    ExitingRegion   = -3002,
    Annotation      = -4000,
    // NOTE: -5000 - -5??? reserved by vsTrackInfo
    QueryAlert      = -10000,
    UserType        = -20000
    };

  //.. Data Members ...........................................................

  Group group;      ///< Event type group.
  int type;         ///< Event type ID.
  QString name;     ///< Event type name.
  vgColor pcolor;   ///< Pen color.
  vgColor bcolor;   ///< Label background color color.
  vgColor fcolor;   ///< Label foreground color.

  /// \var group
  /// If vsEventInfo::Unknown (\c 0), the event type group will be determined
  /// automatically based on the event type ID.

  //.. Utility Methods ........................................................

  vgEventType toVgEventType() const;

  static vsEventInfo fromEventSetInfo(const vvEventSetInfo&, int type = -1);

  /// Read event types.
  ///
  /// This reads a list of event types from the specified QSettings object in
  /// the specified settings group.
  ///
  /// \param settings
  ///   QSettings instance from which to read event types.
  /// \param settingsGroup
  ///   QSettings group where event types are stored.
  /// \param templateArray
  ///   List of event type info templates which define built-in types, or
  ///   \c nullptr. If specified, these will be returned even if no event types
  ///   are defined in \p settings. The end of the array must be indicated by
  ///   an item having Template::type \c 0.
  static QList<vsEventInfo> events(
    QSettings& settings, const QString& settingsGroup,
    const vsEventInfo::Template* templateArray = 0);

  /// Read standard event types.
  ///
  /// This returns a list of standard event types from the default location in
  /// QSettings. If \p groups is not vsEventInfo::All, only the events types
  /// for the specified group(s) are read.
  static QList<vsEventInfo> events(Groups groups = All);

  /// Determine the type group of an event type.
  ///
  /// This returns the appropriate event type Group, given an event type ID and
  /// a group hint. If the hint is not vsEventInfo::Unknown, the hint is
  /// returned.
  static Group eventGroup(int eventType, Group hint = Unknown);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(vsEventInfo::Groups)

#endif
