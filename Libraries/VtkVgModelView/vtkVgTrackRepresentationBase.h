// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTrackRepresentationBase_h
#define __vtkVgTrackRepresentationBase_h

#include "vtkVgRepresentationBase.h"
#include "vtkVgTrack.h"
#include "vtkVgTrackInfo.h"

#include <vector>

#include <vgExport.h>

class vtkActor;
class vtkPoints;
class vtkRenderer;

class vtkVgContourOperatorManager;
class vtkVgTrackFilter;
class vtkVgTrackModel;
class vtkVgTrackTypeRegistry;

class vtkVgTrackColorHelper
{
public:
  virtual ~vtkVgTrackColorHelper() {}
  virtual const double* GetTrackColor(vtkVgTrackInfo track,
                                      double scalar = 0.0) = 0;
};

class VTKVG_MODELVIEW_EXPORT vtkVgTrackRepresentationBase
  : public vtkVgRepresentationBase
{
public:
  enum TrackColorMode
    {
    TCM_Model,      // Use track colors supplied by the model
    TCM_TOC,        // Color based on track object classification
    TCM_Random,     // Random coloring based on track id
    TCM_Scalars,    // Color based on scalars
    TCM_StateAttrs, // Color using track state attributes
    TCM_Default     // Use single default color
    };

public:
  vtkTypeMacro(vtkVgTrackRepresentationBase, vtkVgRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Update the representation in preparation for rendering.
  virtual void Update() = 0;

  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked trackId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account
  // whether the track is currently visible or not.
  virtual vtkIdType Pick(double renX, double renY, vtkRenderer* ren, vtkIdType& pickType);

  // Description:
  // After a successful pick occurs, return the position of the pick (in
  // world coordinates).
  virtual void GetPickPosition(double pos[3]);

  // Description:
  // Set the item on the representation.
  void SetTrackModel(vtkVgTrackModel* trackItem);
  vtkGetObjectMacro(TrackModel, vtkVgTrackModel);

  // Description:
  // Set the track filter that this representation will use.
  virtual void SetTrackFilter(vtkVgTrackFilter* filter);
  vtkGetObjectMacro(TrackFilter, vtkVgTrackFilter);

  // Description:
  // Set the track type registry that this representation will use.
  virtual void SetTrackTypeRegistry(vtkVgTrackTypeRegistry* typeRegistry);
  vtkGetObjectMacro(TrackTypeRegistry, vtkVgTrackTypeRegistry);

  // Description:
  // Set track color helper for the representation. The caller is responsible
  // for managing the life of the helper, and ensuring it is not deleted while
  // the representation holds a reference to it.
  void SetColorHelper(vtkVgTrackColorHelper* colorHelper);
  vtkGetObjectMacro(ColorHelper, vtkVgTrackColorHelper);

  // Description:
  // Set/Get the way track color is determined.
  vtkSetMacro(ColorMode, TrackColorMode);
  vtkGetMacro(ColorMode, TrackColorMode);

  void SetColorModeToModel()  { this->SetColorMode(TCM_Model); }
  void SetColorModeToTOC()    { this->SetColorMode(TCM_TOC); }
  void SetColorModeToRandom() { this->SetColorMode(TCM_Random); }

  [[deprecated]] void SetColorModeToPVO() { this->SetColorMode(TCM_TOC); }

  bool UsingPerFrameColors()
    {
    return this->ColorMode == TCM_Scalars ||
           this->ColorMode == TCM_StateAttrs;
    }

  // Description:
  // Set/Get the group mask to apply when coloring by state attributes
  vtkSetMacro(StateAttributeGroupMask, vtkTypeUInt64);
  vtkGetMacro(StateAttributeGroupMask, vtkTypeUInt64);

  // Description:
  // Add an implicit mapping for an individual attribute mask. Masks should be
  // registered in the same order that their colors appear in the lookup table,
  // starting at the second entry. The color in the first table entry will be
  // used for any attribute combination that hasn't been registered.
  void AddStateAttributeMask(vtkTypeUInt64 mask);
  void ClearStateAttributeMasks();

  // Description:
  // Set/Get the rgb components of the "Person" color (if ColorByTOC).
  // Note: expecting color values between 0 and 1.
  [[deprecated]] vtkSetVector3Macro(PersonColor, double);
  [[deprecated]] vtkGetVector3Macro(PersonColor, double);

  // Description:
  // Set/Get the rgb components of the "Vehicle" color (if ColorByTOC).
  // Note: expecting color values between 0 and 1.
  [[deprecated]] vtkSetVector3Macro(VehicleColor, double);
  [[deprecated]] vtkGetVector3Macro(VehicleColor, double);

  // Description:
  // Set/Get the rgb components of the "Other" color (if ColorByTOC).
  // Note: expecting color values between 0 and 1.
  [[deprecated]] vtkSetVector3Macro(OtherColor, double);
  [[deprecated]] vtkGetVector3Macro(OtherColor, double);

  // Description:
  // Set/Get the rgb components of the "Unclassified" color (if ColorByTOC).
  // Note: expecting color values between 0 and 1.
  vtkSetVector3Macro(UnclassifiedColor, double);
  vtkGetVector3Macro(UnclassifiedColor, double);

  // Description:
  // Set the rgb components of the specified track type color (if ColorByTOC).
  // Note: expecting color values between 0 and 1.
  [[deprecated]]
  void SetColor(vtkVgTrack::enumTrackPVOType type, double color[3]);

  // Description:
  // Set the rgb components of the default track color.
  // Note: expecting color values between 0 and 1.
  vtkSetVector3Macro(DefaultColor, double);
  vtkGetVector3Macro(DefaultColor, double);

  // Description:
  // Set/Get the manager for filters and selectors
  void SetContourOperatorManager(vtkVgContourOperatorManager* manager);
  vtkGetObjectMacro(ContourOperatorManager, vtkVgContourOperatorManager);

  // Description:
  // Set/Get track to exclude from display
  void SetExcludedTrack(vtkVgTrack* track);
  vtkGetObjectMacro(ExcludedTrack, vtkVgTrack);

protected:
  vtkVgTrackRepresentationBase();
  virtual ~vtkVgTrackRepresentationBase();

  int GetTrackTOCType(vtkVgTrack* track);
  const double* GetTrackColor(vtkVgTrackInfo track, double scalar = 0.0);

  void SetNormalizedColor(double memberColor[3], double inputColor[3]);

  vtkVgContourOperatorManager* ContourOperatorManager;

  vtkVgTrackModel* TrackModel;
  vtkVgTrackFilter* TrackFilter;
  vtkVgTrackTypeRegistry* TrackTypeRegistry;

  TrackColorMode   ColorMode;
  double           PersonColor[3];
  double           VehicleColor[3];
  double           OtherColor[3];
  double           UnclassifiedColor[3];
  double           ScalarColor[3];
  double           DefaultColor[3];

  vtkVgTrackColorHelper* ColorHelper;

  vtkTypeUInt64 StateAttributeGroupMask;

  std::vector<vtkTypeUInt64> RegisteredAttributeMasks;

  vtkVgTrack* ExcludedTrack;

private:
  vtkVgTrackRepresentationBase(const vtkVgTrackRepresentationBase&);
  void operator=(const vtkVgTrackRepresentationBase&);
};

#endif // __vtkVgTrackRepresentationBase_h
