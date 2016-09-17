/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvTrackInfo.h"

#include <QColor>
#include <QSettings>

#include <vgColor.h>

namespace // anonymous
{

const char* const settingsTypeRoot = "TrackTypes";
const char* const settingsSourceRoot = "TrackSources";

//-----------------------------------------------------------------------------
struct TrackInfoTemplate
{
  int type;
  const char* group;
  unsigned char color[6];
};

//-----------------------------------------------------------------------------
static const TrackInfoTemplate defaultTrackTemplate =
{
  vtkVgTrack::Unclassified,   "Unclassified",
  { 208, 208, 208, 144, 144, 144 }
};

//-----------------------------------------------------------------------------
static const TrackInfoTemplate nullTrackTemplate =
{
  0, 0, { 0, 0, 0, 0, 0, 0 }
};

//-----------------------------------------------------------------------------
static const TrackInfoTemplate trackTemplates[] =
{
  { vtkVgTrack::Fish,         "Fish",
    {  64,  64, 224,  48,  48, 192 } },
  { vtkVgTrack::Scallop,        "Scallop",
    { 208, 208,  48, 160, 160,  32 } },
  { vtkVgTrack::Other,          "Other",
    { 112, 112, 112,  64,  64,  64 } },
  defaultTrackTemplate,
  nullTrackTemplate
};

//-----------------------------------------------------------------------------
vgColor color(unsigned char* p, int o)
{
  return QColor::fromRgb(p[o + 0], p[o + 1], p[o + 2]);
}

//-----------------------------------------------------------------------------
vvTrackInfo trackFromSettings(
  QSettings& settings, const QString& group, TrackInfoTemplate tpl)
{
  vvTrackInfo ti;

  ti.Type = static_cast<vtkVgTrack::enumTrackPVOType>(tpl.type);

  settings.beginGroup(group);

  ti.Name = settings.value("Name", tpl.group).toString();

  ti.PenColor =
    vgColor::read(settings, "PenColor", color(tpl.color, 0));
  ti.BackgroundColor =
    vgColor::read(settings, "BackgroundColor", color(tpl.color, 3));
  ti.ForegroundColor =
    vgColor::read(settings, "ForegroundColor", vgColor(Qt::white));

  settings.endGroup();

  return ti;
}

//-----------------------------------------------------------------------------
vvTrackInfo trackFromSettings(QSettings& settings, TrackInfoTemplate tpl)
{
  return trackFromSettings(settings, tpl.group, tpl);
}

//-----------------------------------------------------------------------------
QList<vvTrackInfo> tracksFromTemplate(QSettings& settings,
                                      const TrackInfoTemplate* t)
{
  QList<vvTrackInfo> tracks;
  while (t->group)
    {
    tracks.append(trackFromSettings(settings, *t));
    ++t;
    }
  return tracks;
}

//-----------------------------------------------------------------------------
const char* settingsGroup(vtkVgTrack::enumTrackPVOType type)
{
  TrackInfoTemplate const* t = trackTemplates;
  while (t->group)
    {
    if (t->type == type)
      {
      return t->group;
      }
    ++t;
    }
  return 0;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vvTrackInfo::vvTrackInfo() : Source(-1), Type(vtkVgTrack::Unclassified)
{
}

//-----------------------------------------------------------------------------
vvTrackInfo::vvTrackInfo(vtkVgTrack::enumTrackPVOType type) :
  Source(-1), Type(type)
{
  vvTrackInfo::getTrackType(*this, type);
}

//-----------------------------------------------------------------------------
bool vvTrackInfo::write() const
{
  QSettings settings;
  if (this->Source >= 0)
    {
    settings.beginGroup(settingsSourceRoot);
    settings.beginGroup(QString::number(this->Source));
    }
  else
    {
    settings.beginGroup(settingsTypeRoot);
    settings.beginGroup(settingsGroup(this->Type));

    settings.setValue("Name", this->Name);
    }

  this->PenColor.write(settings, "PenColor");
  this->BackgroundColor.write(settings, "BackgroundColor");
  this->ForegroundColor.write(settings, "ForegroundColor");

  return true;
}

//-----------------------------------------------------------------------------
bool vvTrackInfo::getTrackType(
  vvTrackInfo& ti, vtkVgTrack::enumTrackPVOType type)
{
  QSettings settings;
  settings.beginGroup(settingsTypeRoot);

  TrackInfoTemplate const* t = trackTemplates;
  while (t->group)
    {
    if (t->type == type)
      {
      ti = trackFromSettings(settings, *t);
      return true;
      }
    ++t;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vvTrackInfo::getTrackSource(vvTrackInfo& ti, int source)
{
  QSettings settings;
  settings.beginGroup(settingsSourceRoot);

  const QString sourceKey = QString::number(source);
  if (settings.contains(sourceKey))
    {
    ti = trackFromSettings(settings, sourceKey, defaultTrackTemplate);
    ti.Source = source;
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
QList<vvTrackInfo> vvTrackInfo::trackTypes()
{
  QSettings settings;
  settings.beginGroup(settingsTypeRoot);

  return tracksFromTemplate(settings, trackTemplates);
}

//-----------------------------------------------------------------------------
QList<vvTrackInfo> vvTrackInfo::trackSources()
{
  QList<vvTrackInfo> tracks;

  QSettings settings;
  settings.beginGroup(settingsSourceRoot);

  foreach (const QString& group, settings.childGroups())
    {
    bool isInt;
    int source = group.toInt(&isInt);
    if (isInt)
      {
      vvTrackInfo ti = trackFromSettings(settings, group, defaultTrackTemplate);
      ti.Source = source;
      tracks.append(ti);
      }
    }
  return tracks;
}

//-----------------------------------------------------------------------------
void vvTrackInfo::eraseTrackSource(int source)
{
  QSettings settings;
  settings.beginGroup(settingsSourceRoot);
  settings.remove(QString::number(source));
}
