/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgVideoRepresentationBase0_h
#define __vtkVgVideoRepresentationBase0_h

// VTK includes.

// VG includes.
#include "vtkVgRepresentationBase.h"

#include <vgExport.h>

// Forward declarations.
class vtkVgEventRepresentationBase;
class vtkVgTrackRepresentationBase;
class vtkVgVideoModel0;

class VTKVG_MODELVIEW_EXPORT vtkVgVideoRepresentationBase0
  : public vtkVgRepresentationBase
{
public:
  vtkVgClassMacro(vtkVgVideoRepresentationBase0);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgVideoRepresentationBase0, vtkVgRepresentationBase);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the item on the representation.
  void  SetVideoModel(vtkVgVideoModel0* videoModel);
  vtkVgVideoModel0* GetVideoModel();
  const vtkVgVideoModel0* GetVideoModel() const;

  void SetTrackRepresentation(vtkVgTrackRepresentationBase* trackRep);
  vtkGetObjectMacro(TrackRepresentation, vtkVgTrackRepresentationBase);

  void SetEventRepresentation(vtkVgEventRepresentationBase* eventRep);
  vtkGetObjectMacro(EventRepresentation, vtkVgEventRepresentationBase);

  virtual void SetEventVisible(int flag) = 0;
  virtual int GetEventVisible() = 0;

  virtual void SetTrackVisible(int flag) = 0;
  virtual int GetTrackVisible() = 0;

protected:
  vtkVgVideoRepresentationBase0();
  virtual ~vtkVgVideoRepresentationBase0();

  int Visible;

  vtkSmartPointer<vtkVgVideoModel0>  VideoModel;
  vtkVgEventRepresentationBase*      EventRepresentation;
  vtkVgTrackRepresentationBase*      TrackRepresentation;

private:
  vtkVgVideoRepresentationBase0(const vtkVgVideoRepresentationBase0&); // Not implemented.
  void operator= (const vtkVgVideoRepresentationBase0&);               // Not implemented.
};

#endif
