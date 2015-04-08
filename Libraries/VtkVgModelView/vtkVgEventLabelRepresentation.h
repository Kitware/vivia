/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgEventLabelRepresentation_h
#define __vtkVgEventLabelRepresentation_h

#include "vtkSmartPointer.h"
#include "vtkVgEventRepresentationBase.h"

#include <vgExport.h>

class vtkCollection;
class vtkMatrix4x4;
class vtkPropCollection;
class vtkRenderer;

class vtkVgEvent;

class VTKVG_MODELVIEW_EXPORT vtkVgEventLabelRepresentation
  : public vtkVgEventRepresentationBase
{
public:
  vtkTypeMacro(vtkVgEventLabelRepresentation, vtkVgEventRepresentationBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVgEventLabelRepresentation* New();

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
  // Set if the label should include the event ID
  vtkSetMacro(ShowId, bool);
  vtkGetMacro(ShowId, bool);

  // Description:
  // Set how many classifiers (max) the label should show
  vtkSetMacro(ShowClassifiers, unsigned int);
  vtkGetMacro(ShowClassifiers, unsigned int);

  // Description:
  // Set if the label should include the classifier score
  vtkSetMacro(ShowProbability, bool);
  vtkGetMacro(ShowProbability, bool);

  // Description:
  // Set if the label should include the event note
  vtkSetMacro(ShowNote, bool);
  vtkGetMacro(ShowNote, bool);

  // Description:
  // Update all the event actors. Generally called by the application layer.
  virtual void Update();

  // Description:
  // Set/Get prefix to add to the label
  vtkSetStringMacro(LabelPrefix);
  vtkGetStringMacro(LabelPrefix);

  enum enumLocationSourceMode
    {
    LSM_Track,            // Place based on track (if present)
    LSM_Region,           // Place based on region (if present)
    LSM_TrackThenRegion,  // Place based on track, if available; region if not
    LSM_RegionThenTrack   // Place based on region, if available; track if not
    };

  // Description:
  // Set/Get the source (track and/or region) for location of the label
  vtkSetMacro(LocationSourceMode, enumLocationSourceMode);
  vtkGetMacro(LocationSourceMode, enumLocationSourceMode);
  void SetLocationSourceModeToTrack()
    {
    this->SetLocationSourceMode(LSM_Track);
    }
  void SetLocationSourceModeToRegion()
    {
    this->SetLocationSourceMode(LSM_Region);
    }
  void SetLocationSourceModeToTrackThenRegion()
    {
    this->SetLocationSourceMode(LSM_TrackThenRegion);
    }
  void SetLocationSourceModeToRegionThenTrack()
    {
    this->SetLocationSourceMode(LSM_RegionThenTrack);
    }

protected:
  virtual void EventRemoved(vtkObject* caller, unsigned long invokedEventId,
                            void* theEvent);

private:
  vtkVgEventLabelRepresentation(const vtkVgEventLabelRepresentation&);
  void operator=(const vtkVgEventLabelRepresentation&);

  vtkVgEventLabelRepresentation();
  virtual ~vtkVgEventLabelRepresentation();

  bool GetLabelPosition(vtkVgEvent* event, double labelPosition[3]);
  bool GetPositionBasedOnRegion(vtkVgEvent* event, double labelPosition[3]);
  bool GetPositionBasedOnTrack(vtkVgEvent* event, double labelPosition[3]);
  void ShowEventAnnotation(vtkVgEvent* event, double labelPosition[3],
                           bool rebuild);
  void HideEventAnnotation(vtkVgEvent* event);

  typedef vtkSmartPointer<vtkPropCollection> vtkPropCollectionRef;

  vtkPropCollectionRef  NewPropCollection;
  vtkPropCollectionRef  ActivePropCollection;
  vtkPropCollectionRef  ExpirePropCollection;

  int          Visible;
  bool         ShowId;
  unsigned int ShowClassifiers;
  bool         ShowProbability;
  bool         ShowNote;
  char*        LabelPrefix;
  enumLocationSourceMode LocationSourceMode;

  struct vtkInternal;
  vtkInternal* Internal;
};

#endif // __vtkVgEventLabelRepresentation_h
