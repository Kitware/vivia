// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTrackRepresentationBase.h"

#include "vtkVgContourOperatorManager.h"
#include "vtkVgPickData.h"
#include "vtkVgPicker.h"
#include "vtkVgTrackFilter.h"
#include "vtkVgTrackModel.h"
#include "vtkVgTrackTypeRegistry.h"

#include <vtkCommand.h>
#include <vtkLookupTable.h>

vtkCxxSetObjectMacro(vtkVgTrackRepresentationBase,
                     TrackFilter,
                     vtkVgTrackFilter);
vtkCxxSetObjectMacro(vtkVgTrackRepresentationBase,
                     TrackTypeRegistry,
                     vtkVgTrackTypeRegistry);
vtkCxxSetObjectMacro(vtkVgTrackRepresentationBase,
                     ContourOperatorManager,
                     vtkVgContourOperatorManager);
vtkCxxSetObjectMacro(vtkVgTrackRepresentationBase,
                     ExcludedTrack,
                     vtkVgTrack);

double RandomColors[][3] =
  {
    {  0.0,  1.0,  1.0 }, // Cyan
    { .941, .902, .549 }, // Khaki
    { .698, .133, .133 }, // Firebrick
    { .416, .353, .980 }, // Slate Blue
    {  1.0,  0.0,  0.0 }, // Red
    {  0.0, .392,  0.0 }, // Dark Green
    { .737, .561, .561 }, // Rosy Brown
    {  0.0,  0.0,  1.0 }, // Blue
    { .498,  1.0,  0.0 }, // Chartreuse
    { .816, .125, .565 }, // Violet Red
    {  1.0, .271,  0.0 }  // Orange Red
  };

enum { NumRandomColors = sizeof(RandomColors) / sizeof(RandomColors[0]) };

//-----------------------------------------------------------------------------
vtkVgTrackRepresentationBase::vtkVgTrackRepresentationBase()
  : vtkVgRepresentationBase(), ContourOperatorManager(0), TrackModel(0),
    TrackFilter(0), TrackTypeRegistry(0), ColorMode(TCM_Model), ColorHelper(0),
    StateAttributeGroupMask(0), ExcludedTrack(0)
{
  // for now, PVO colors default to P=b, V=g, O=r
  this->PersonColor[0] = 0.0;
  this->PersonColor[1] = 0.0;
  this->PersonColor[2] = 1.0;
  this->VehicleColor[0] = 0.0;
  this->VehicleColor[1] = 1.0;
  this->VehicleColor[2] = 0.0;
  this->OtherColor[0] = 1.0;
  this->OtherColor[1] = 0.0;
  this->OtherColor[2] = 0.0;
  this->UnclassifiedColor[0] = 1.0;
  this->UnclassifiedColor[1] = 1.0;
  this->UnclassifiedColor[2] = 0.0;
  this->ScalarColor[0] = 0.0;
  this->ScalarColor[1] = 0.0;
  this->ScalarColor[2] = 0.0;
  this->DefaultColor[0] = 1.0;
  this->DefaultColor[1] = 1.0;
  this->DefaultColor[2] = 1.0;
  this->SetLayerIndex(1);
}

//-----------------------------------------------------------------------------
vtkVgTrackRepresentationBase::~vtkVgTrackRepresentationBase()
{
  this->SetTrackFilter(0);
  this->SetTrackTypeRegistry(0);
  this->SetTrackModel(0);
  this->SetContourOperatorManager(0);
  this->SetExcludedTrack(0);
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrackRepresentationBase::Pick(double vtkNotUsed(renX),
                                             double vtkNotUsed(renY),
                                             vtkRenderer* vtkNotUsed(ren),
                                             vtkIdType& pickType)
{
  pickType = vtkVgPickData::EmptyPick;
  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentationBase::GetPickPosition(double vtkNotUsed(pos)[])
{
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentationBase::SetTrackModel(vtkVgTrackModel* trackModel)
{
  if (this->TrackModel != trackModel)
    {
    vtkVgTrackModel* temp = this->TrackModel;
    this->TrackModel = trackModel;
    if (this->TrackModel != NULL)
      {
      this->TrackModel->Register(this);
      }
    if (temp != NULL)
      {
      temp->UnRegister(this);
      }

    if (this->TrackModel && this->UseAutoUpdate)
      {
      this->TrackModel->AddObserver(vtkCommand::UpdateDataEvent, this,
                                    &vtkVgTrackRepresentationBase::Update);
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentationBase::AddStateAttributeMask(vtkTypeUInt64 mask)
{
  this->RegisteredAttributeMasks.push_back(mask);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentationBase::ClearStateAttributeMasks()
{
  this->RegisteredAttributeMasks.clear();
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentationBase::SetColor(vtkVgTrack::enumTrackPVOType type,
                                            double color[3])
{
  switch (type)
    {
    case vtkVgTrack::Person:
      this->SetPersonColor(color);
      break;
    case vtkVgTrack::Vehicle:
      this->SetVehicleColor(color);
      break;
    case vtkVgTrack::Other:
      this->SetOtherColor(color);
      break;
    default:
      this->SetUnclassifiedColor(color);
      break;
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrackRepresentationBase::SetColorHelper(
  vtkVgTrackColorHelper* colorHelper)
{
  if (this->ColorHelper != colorHelper)
    {
    this->ColorHelper = colorHelper;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
int vtkVgTrackRepresentationBase::GetTrackTOCType(vtkVgTrack* track)
{
  if (this->TrackFilter)
    {
    return this->TrackFilter->GetBestClassifier(track);
    }
  return track->GetBestTOCClassifier().first;
}

//-----------------------------------------------------------------------------
const double* vtkVgTrackRepresentationBase::GetTrackColor(
  vtkVgTrackInfo trackInfo, double scalar)
{
  if (this->HasOverrideColor)
    {
    return this->OverrideColor;
    }

  vtkVgTrack* const track = trackInfo.GetTrack();
  if (track->GetUseCustomColor())
    {
    return track->GetCustomColor();
    }

  if (this->ColorHelper)
    {
    const double* const customColor =
      this->ColorHelper->GetTrackColor(trackInfo, scalar);
    if (customColor)
      {
      return customColor;
      }
    }

  switch (this->ColorMode)
    {
    case TCM_Model:
      return track->GetColor();

    case TCM_TOC:
      {
      if (!trackInfo.GetDisplayTrack())
        {
        return this->UnclassifiedColor;
        }

      const auto tt = this->GetTrackTOCType(track);
      if (this->TrackTypeRegistry)
        {
        if (tt >= 0 && tt < this->TrackTypeRegistry->GetNumberOfTypes())
          {
          const auto& type = this->TrackTypeRegistry->GetEntityType(tt);
          return type.GetColor();
          }
        }
      else
        {
        switch (tt)
          {
          case vtkVgTrack::Person:  return this->PersonColor;
          case vtkVgTrack::Vehicle: return this->VehicleColor;
          case vtkVgTrack::Other:   return this->OtherColor;
          default:                  break;
          }
        }
      return this->UnclassifiedColor;
      }

    case TCM_Random:
      return RandomColors[track->GetId() % NumRandomColors];

    case TCM_StateAttrs:
      {
      vtkTypeUInt64 bits = static_cast<vtkTypeUInt64>(scalar + 0.5);
      bits &= this->StateAttributeGroupMask;

      // if there are no active attributes in this group, use the normal track
      // model color
      if (bits == 0)
        {
        return track->GetColor();
        }

      // unknown attributes are mapped as scalar 0
      scalar = 0.0;

      // map from attribute mask to LUT index
      for (size_t i = 0, size = this->RegisteredAttributeMasks.size(); i < size;
           ++i)
        {
        if (bits == this->RegisteredAttributeMasks[i])
          {
          scalar = static_cast<double>(i + 1);
          break;
          }
        }

      // fall through
      }

    case TCM_Scalars:
      {
      if (this->LookupTable)
        {
        this->LookupTable->GetColor(scalar, this->ScalarColor);
        return this->ScalarColor;
        }
      else
        {
        vtkErrorMacro("<< No lookup table is found \n");
        }
      }

    case TCM_Default:
      {
      return this->DefaultColor;
      }
    }

  return 0;
}
