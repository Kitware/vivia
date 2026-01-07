// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTrackLabelRepresentation_h
#define __vtkVgTrackLabelRepresentation_h

#include "vtkSmartPointer.h"
#include "vtkVgTrackRepresentationBase.h"

#include <vgExport.h>

class vtkPropCollection;
class vtkVgTrack;

class vtkVgTrackLabelColorHelper
{
public:
  virtual ~vtkVgTrackLabelColorHelper() {}
  virtual const double* GetTrackLabelBackgroundColor(vtkVgTrack* track) = 0;
  virtual const double* GetTrackLabelForegroundColor(vtkVgTrack* track) = 0;
};

class VTKVG_MODELVIEW_EXPORT vtkVgTrackLabelRepresentation
  : public vtkVgTrackRepresentationBase
{
public:
  vtkTypeMacro(vtkVgTrackLabelRepresentation, vtkVgTrackRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgTrackLabelRepresentation* New();

  // Description:
  // Set the foreground and background label colors for a given track type.
  int SetTrackTypeColors(const int id,
                         const double bcolor[3] = 0,
                         const double fcolor[3] = 0);

  // Description:
  // Set track label color helper for the representation. The caller is
  // responsible for managing the life of the helper, and ensuring it is not
  // deleted while the representation holds a reference to it.
  void SetLabelColorHelper(vtkVgTrackLabelColorHelper* labelColorHelper);
  vtkGetObjectMacro(LabelColorHelper, vtkVgTrackLabelColorHelper);

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

  virtual vtkIdType Pick(double renX,
                         double renY,
                         vtkRenderer* ren,
                         vtkIdType& pickType);

  // Description:
  // Set if the label should include the track name (will show ID if name unset)
  vtkSetMacro(ShowName, bool);
  vtkGetMacro(ShowName, bool);

  // Description:
  // Set if the label should include the classifier score
  vtkSetMacro(ShowProbability, bool);
  vtkGetMacro(ShowProbability, bool);

  // Description:
  // Set if the label should include the track note
  vtkSetMacro(ShowNote, bool);
  vtkGetMacro(ShowNote, bool);

  // Description:
  // Set/Get prefix to add to the label
  vtkSetStringMacro(LabelPrefix);
  vtkGetStringMacro(LabelPrefix);

  // Description:
  // Update all the track actors. Generally called by the application layer.
  virtual void Update();

private:
  vtkVgTrackLabelRepresentation(const vtkVgTrackLabelRepresentation&);
  void operator=(const vtkVgTrackLabelRepresentation&);

  vtkVgTrackLabelRepresentation();
  virtual ~vtkVgTrackLabelRepresentation();

  void ShowTrackAnnotation(vtkVgTrack* track, bool rebuild);
  void HideTrackAnnotation(vtkVgTrack* track);

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  vtkVgTrackLabelColorHelper* LabelColorHelper;

  int          Visible;
  bool         ShowName;
  bool         ShowProbability;
  bool         ShowNote;
  char*        LabelPrefix;

  struct vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgTrackLabelRepresentation_h
