// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsTrackInfo.h"

#include <QHash>

#include <vgColor.h>

#include <vtkVgTrack.h>

#include <vvTrackInfo.h>

const int vsTrackInfo::GroundTruthSource = 1000;

namespace // anonymous
{

//-----------------------------------------------------------------------------
vsTrackInfo adapt(const vvTrackInfo& in)
{
  vsTrackInfo out;

  out.name = in.Name;
  out.type = in.Type;
  out.source = in.Source;
  in.PenColor.fillArray(out.pcolor);
  in.ForegroundColor.fillArray(out.fcolor);
  in.BackgroundColor.fillArray(out.bcolor);

  return out;
}

//-----------------------------------------------------------------------------
void adapt(
  QList<vsTrackInfo>& out,
  QHash<vtkVgTrack::enumTrackPVOType, vvTrackInfo> const& src,
  vtkVgTrack::enumTrackPVOType vtkType,
  vsTrackInfo::Type vsType)
{
  if (!src.contains(vtkType))
    return;

  vsTrackInfo ati = adapt(src[vtkType]);
  ati.id = vsType;

  out.append(ati);
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
QList<vsTrackInfo> vsTrackInfo::trackTypes()
{
  QHash<vtkVgTrack::enumTrackPVOType, vvTrackInfo> types;
  foreach (vvTrackInfo ti, vvTrackInfo::trackTypes())
    types.insert(ti.Type, ti);

  QList<vsTrackInfo> result;
  adapt(result, types, vtkVgTrack::Person,       vsTrackInfo::Person);
  adapt(result, types, vtkVgTrack::Vehicle,      vsTrackInfo::Vehicle);
  adapt(result, types, vtkVgTrack::Other,        vsTrackInfo::Other);
  adapt(result, types, vtkVgTrack::Unclassified, vsTrackInfo::Unclassified);

  return result;
}

//-----------------------------------------------------------------------------
QList<vsTrackInfo> vsTrackInfo::trackSources()
{
  QList<vsTrackInfo> result;
  foreach (vvTrackInfo ti, vvTrackInfo::trackSources())
    {
    vsTrackInfo ati = adapt(ti);
    ati.id = ti.Source;
    result.append(ati);
    }

  return result;
}
