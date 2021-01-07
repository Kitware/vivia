// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgModelBase_h
#define __vtkVgModelBase_h

// VTK includes.
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// VG includes.
#include <vtkVgMacros.h>
#include "vtkVgTimeStamp.h"

#include <vgExport.h>

// Forward declarations.
class vtkMatrix4x4;
class vtkVgContourOperatorManager;
class vtkVgTemporalFilters;

class VTKVG_MODELVIEW_EXPORT vtkVgModelBase : public vtkObject
{
public:
  vtkVgClassMacro(vtkVgModelBase);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgModelBase, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  vtkSetMacro(Id, vtkIdType);
  vtkGetMacro(Id, vtkIdType);

  // Description:
  vtkSetMacro(UseInternalTimeStamp, int);
  vtkGetMacro(UseInternalTimeStamp, int);
  vtkBooleanMacro(UseInternalTimeStamp, int);

  // Description:
  // This is model specific matrix. This could be anything, a matrix
  // that puts the  model in world space or a homography matrix.
  void SetModelMatrix(vtkMatrix4x4* matrix);
  const vtkMatrix4x4* GetModelMatrix() const;
  vtkMatrix4x4* GetModelMatrix();

  // Description:
  // Return the MTime for the UpdateDataRequest of the model
  unsigned long GetUpdateDataRequestTime()
    {
    return this->UpdateDataRequestTime.GetMTime();
    }

  virtual int Update(
    const vtkVgTimeStamp& vtkNotUsed(timeStamp),
    const vtkVgTimeStamp* vtkNotUsed(referenceFrameTimeStamp))
    { return VTK_OK; }

  // Overloads required for Python wrapping, which does not support pointer to
  // vtkVgTimeStamp as a parameter type
  int Update(const vtkVgTimeStamp& timeStamp)
    { return this->Update(timeStamp, 0); }
  int Update(const vtkVgTimeStamp& timeStamp,
             const vtkVgTimeStamp& referenceFrameTimeStamp)
    { return this->Update(timeStamp, &referenceFrameTimeStamp); }

  // Description:
  // Return current time stamp.
  const vtkVgTimeStamp& GetCurrentTimeStamp() const
    {
    return this->CurrentTimeStamp;
    }

  // Description:
  // Return the MTime for the last Update (that did something).
  virtual unsigned long GetUpdateTime() = 0;

  // Set/Get the manager for filters and selectors
  void SetContourOperatorManager(vtkVgContourOperatorManager* manager);
  vtkGetObjectMacro(ContourOperatorManager, vtkVgContourOperatorManager);

  // Set/Get the manager for temporal filters
  void SetTemporalFilters(vtkVgTemporalFilters* temporalFilters);
  vtkGetObjectMacro(TemporalFilters, vtkVgTemporalFilters);

protected:
  vtkVgModelBase();
  virtual ~vtkVgModelBase();

  // Description:
  // Set/Get flag if the model is initialized
  vtkSetMacro(Initialized, int);
  vtkGetMacro(Initialized, int)
  vtkBooleanMacro(Initialized, int);

// Description:
  vtkIdType                      Id;

// Description:
// The MTime of an update data request
  vtkTimeStamp                   UpdateDataRequestTime;

// Description:
// Flag to allow model to use its internal timestamp.
  int                            UseInternalTimeStamp;

  int                            Initialized;

  vtkSmartPointer<vtkMatrix4x4>  ModelMatrix;

// Description:
// The Current timestamp
  vtkVgTimeStamp                 CurrentTimeStamp;

  vtkVgContourOperatorManager*   ContourOperatorManager;
  vtkVgTemporalFilters* TemporalFilters;

private:
  vtkVgModelBase(const vtkVgModelBase&);    // Not implemented.
  void operator= (const vtkVgModelBase&);   // Not implemented.
};

#endif // __vtkVgModelBase_h
