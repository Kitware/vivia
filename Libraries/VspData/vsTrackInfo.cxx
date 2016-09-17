/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
  adapt(result, types, vtkVgTrack::Fish,         vsTrackInfo::Fish);
  adapt(result, types, vtkVgTrack::Scallop,      vsTrackInfo::Scallop);
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
