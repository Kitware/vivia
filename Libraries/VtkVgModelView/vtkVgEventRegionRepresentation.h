// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgEventRegionRepresentation_h
#define __vtkVgEventRegionRepresentation_h

#include "vtkSmartPointer.h"
#include "vtkVgEventRepresentationBase.h"

#include <vgExport.h>

class vtkVgCellPicker;

class vtkIdList;
class vtkPropCollection;

class VTKVG_MODELVIEW_EXPORT vtkVgEventRegionRepresentation
  : public vtkVgEventRepresentationBase
{
public:
  enum ExclusionMode
    {
    EM_ExcludeNone,
    EM_ExcludeMarked,
    EM_ExcludeUnmarked
    };

public:
  // Description:
  // Easy to use.
  vtkVgClassMacro(vtkVgEventRegionRepresentation);

  vtkTypeMacro(vtkVgEventRegionRepresentation, vtkVgEventRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgEventRegionRepresentation* New();

  // Description:
  // Return all the objects that can be rendered.
  virtual const vtkPropCollection* GetNewRenderObjects() const;
  virtual const vtkPropCollection* GetActiveRenderObjects() const;
  virtual const vtkPropCollection* GetExpiredRenderObjects() const;

  virtual vtkPropCollection* GetNewRenderObjects();
  virtual vtkPropCollection* GetActiveRenderObjects();
  virtual vtkPropCollection* GetExpiredRenderObjects();

  virtual void ResetTemporaryRenderObjects();

  // Description:
  virtual void SetVisible(int flag);
  virtual int  GetVisible() const { return this->Visible; }

  virtual float GetLineWidth() const;
  virtual void SetLineWidth(float value);

  // Description:
  // Update all the event actors. Generally called by the application layer.
  virtual void Update();

  virtual vtkIdType Pick(double renX, double renY, vtkRenderer* ren,
                         vtkIdType& pickType);

  // Description:
  // Set/Get the Z offset to be applied to the region actor.
  // Note: the Z_OFFSET of the prop will be added to get final Z offset
  vtkSetMacro(RegionZOffset, double);
  vtkGetMacro(RegionZOffset, double);

  void SetColor(double r, double g, double b);

private:
  vtkVgEventRegionRepresentation(const vtkVgEventRegionRepresentation&);
  void operator=(const vtkVgEventRegionRepresentation&);

  vtkVgEventRegionRepresentation();
  virtual ~vtkVgEventRegionRepresentation();

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;
  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  int      Visible;
  double   RegionZOffset;

  struct vtkInternal;
  vtkInternal* Internal;

  vtkSmartPointer<vtkVgCellPicker> Picker;
};

#endif // __vtkVgEventRegionRepresentation_h
