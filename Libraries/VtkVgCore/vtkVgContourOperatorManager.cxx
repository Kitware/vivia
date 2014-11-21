/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgContourOperatorManager.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkImplicitBoolean.h"
#include "vtkImplicitSelectionLoop.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"
#include "vtkSmartPointer.h"

#include <map>

vtkStandardNewMacro(vtkVgContourOperatorManager);

struct vtkVgContourInfo
{
  vtkImplicitSelectionLoop* Loop;
  bool Enabled;

  vtkVgContourInfo()
    {
    this->Loop = 0;
    this->Enabled = true;
    }

  ~vtkVgContourInfo()
    {
    if (this->Loop)
      {
      this->Loop->UnRegister(NULL);
      this->Loop = 0;
      }
    }

  // Function ensures proper reference counting.
  void SetLoop(vtkImplicitSelectionLoop* loop)
    {
    if (this->Loop != loop)
      {
      // Before assigning a new one store the last one
      // so that we can decrement the ref count later.
      vtkImplicitSelectionLoop* temp = this->Loop;
      this->Loop = loop;
      // Increment the ref count.
      if (this->Loop != NULL)
        {
        this->Loop->Register(NULL);
        }
      if (temp != NULL)
        {
        // Decrement the ref count.
        temp->UnRegister(NULL);
        }
      }
    }

  vtkVgContourInfo(const vtkVgContourInfo& fromContourInfo)
    {
    this->Enabled = fromContourInfo.Enabled;
    this->Loop = 0;
    this->SetLoop(fromContourInfo.Loop);
    }

  vtkVgContourInfo& operator=(const vtkVgContourInfo& fromContourInfo)
    {
    this->Enabled = fromContourInfo.Enabled;
    this->Loop = 0;
    this->SetLoop(fromContourInfo.Loop);
    return *this;
    }

};

//----------------------------------------------------------------------------
class vtkVgContourOperatorManager::vtkInternal
{
public:
  vtkInternal()
    {
    }

  ~vtkInternal()
    {
    this->RemoveAllContours(this->Filters);
    this->RemoveAllContours(this->Selectors);
    }

  bool AddContour(vtkPoints* loopPoints, vtkImplicitBoolean* boolean,
                  std::map<vtkPoints*, vtkVgContourInfo>& mainMap,
                  std::map<vtkPoints*, vtkVgContourInfo>& otherMap);
  bool RemoveContour(vtkPoints* loopPoints, vtkImplicitBoolean* boolean,
                     std::map<vtkPoints*, vtkVgContourInfo>& theMap);
  bool RemoveAllContours(
    std::map<vtkPoints*, vtkVgContourInfo>& theMap,
    vtkImplicitBoolean* boolean = 0);
  bool SetContourEnabled(vtkPoints* loopPoints, bool state,
    vtkImplicitBoolean* filterBoolean, vtkImplicitBoolean* selectorBoolean);
  bool GetContourEnabled(vtkPoints* loopPoints);
  int GetNumberOfEnabledContours(
    std::map<vtkPoints*, vtkVgContourInfo>& theMap);
  unsigned long GetContoursMTime(
    std::map<vtkPoints*, vtkVgContourInfo>& theMap);

  void UpdateBoolean(vtkImplicitBoolean* boolean,
                     vtkImplicitSelectionLoop* selectorLoop, bool addFunction);

  vtkImplicitSelectionLoop* GetContourLoop(vtkPoints* loopPoints);

  std::map<vtkPoints*, vtkVgContourInfo> Filters;
  std::map<vtkPoints*, vtkVgContourInfo> Selectors;
};


//----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::vtkInternal::AddContour(vtkPoints* loopPoints,
    vtkImplicitBoolean* boolean,
    std::map<vtkPoints*, vtkVgContourInfo>& mainMap,
    std::map<vtkPoints*, vtkVgContourInfo>& otherMap)
{
  if (otherMap.find(loopPoints) != otherMap.end())
    {
    vtkGenericWarningMacro("Can not add; already added to another list!");
    return false;
    }

  if (mainMap.find(loopPoints) != mainMap.end())
    {
    return false;  // already added
    }

  vtkVgContourInfo contourInfo;  // defaults to enablesd
  vtkImplicitSelectionLoop* loop = vtkImplicitSelectionLoop::New();
  loop->SetLoop(loopPoints);
  contourInfo.SetLoop(loop);
  mainMap[loopPoints] = contourInfo;
  loop->FastDelete();

  // that said, could potentially let the boolean hold onto the reference?
  boolean->AddFunction(loop);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::vtkInternal::RemoveContour(
    vtkPoints* loopPoints, vtkImplicitBoolean* boolean, std::map<vtkPoints*,
    vtkVgContourInfo>& theMap)
{
  std::map<vtkPoints*, vtkVgContourInfo>::iterator iter =
    theMap.find(loopPoints);
  if (iter == theMap.end())
    {
    return false;
    }

  if (iter->second.Enabled)
    {
    boolean->RemoveFunction(iter->second.Loop);
    }
  theMap.erase(iter);
  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::vtkInternal::RemoveAllContours(
  std::map<vtkPoints*, vtkVgContourInfo>& theMap,
  vtkImplicitBoolean* boolean/*=0*/)
{
  if (theMap.size())
    {
    std::map<vtkPoints*, vtkVgContourInfo>::const_iterator iter;
    for (iter = theMap.begin(); iter != theMap.end(); iter++)
      {
      if (boolean && iter->second.Enabled)
        {
        boolean->RemoveFunction(iter->second.Loop);
        }
      }
    theMap.clear();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::vtkInternal::
SetContourEnabled(vtkPoints* loopPoints, bool state,
                  vtkImplicitBoolean* filterBoolean,
                  vtkImplicitBoolean* selectorBoolean)
{
  vtkImplicitBoolean* boolean = filterBoolean;
  std::map<vtkPoints*, vtkVgContourInfo>::iterator iter =
    this->Filters.find(loopPoints);
  if (iter == this->Filters.end())
    {
    iter = this->Selectors.find(loopPoints);
    if (iter == this->Selectors.end())
      {
      return false;
      }
    boolean = selectorBoolean;
    }

  if (iter->second.Enabled == state)
    {
    return false;
    }

  iter->second.Enabled = state;
  if (state)
    {
    boolean->AddFunction(iter->second.Loop);
    }
  else
    {
    boolean->RemoveFunction(iter->second.Loop);
    }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::vtkInternal::
GetContourEnabled(vtkPoints* loopPoints)
{
  std::map<vtkPoints*, vtkVgContourInfo>::iterator iter =
    this->Filters.find(loopPoints);
  if (iter == this->Filters.end())
    {
    iter = this->Selectors.find(loopPoints);
    if (iter == this->Selectors.end())
      {
      return false;
      }
    }

  return iter->second.Enabled;
}

//-----------------------------------------------------------------------------
int vtkVgContourOperatorManager::vtkInternal::
GetNumberOfEnabledContours(std::map<vtkPoints*, vtkVgContourInfo>& theMap)
{
  int count = 0;
  std::map<vtkPoints*, vtkVgContourInfo>::const_iterator iter;
  for (iter = theMap.begin(); iter != theMap.end(); iter++)
    {
    if (iter->second.Enabled)
      {
      count++;
      }
    }
  return count;
}

//-----------------------------------------------------------------------------
unsigned long vtkVgContourOperatorManager::vtkInternal::GetContoursMTime(
  std::map<vtkPoints*, vtkVgContourInfo>& theMap)
{
  unsigned long time, mTime = 0;
  std::map<vtkPoints*, vtkVgContourInfo>::const_iterator iter;
  for (iter = theMap.begin(); iter != theMap.end(); iter++)
    {
    time = iter->first->GetMTime();
    mTime = time > mTime ? time : mTime;
    time = iter->second.Loop->GetMTime();
    mTime = time > mTime ? time : mTime;
    }

  return mTime;
}

//-----------------------------------------------------------------------------
void vtkVgContourOperatorManager::vtkInternal::UpdateBoolean(
  vtkImplicitBoolean* boolean, vtkImplicitSelectionLoop* selectorLoop,
  bool addFunction)
{
  if (addFunction)
    {
    boolean->AddFunction(selectorLoop);
    }
  else
    {
    boolean->RemoveFunction(selectorLoop);
    }
}

//-----------------------------------------------------------------------------
vtkImplicitSelectionLoop*
vtkVgContourOperatorManager::vtkInternal::GetContourLoop(vtkPoints* loopPoints)
{
  std::map<vtkPoints*, vtkVgContourInfo>::iterator iter =
    this->Filters.find(loopPoints);
  if (iter == this->Filters.end())
    {
    iter = this->Selectors.find(loopPoints);
    if (iter == this->Selectors.end())
      {
      return 0;
      }
    }
  return iter->second.Loop;
}

//----------------------------------------------------------------------------
vtkVgContourOperatorManager::vtkVgContourOperatorManager()
{
  this->FilterBoolean = vtkImplicitBoolean::New();
  this->FilterBoolean->SetOperationTypeToUnion();
  this->SelectorBoolean = vtkImplicitBoolean::New();
  this->SelectorBoolean->SetOperationTypeToUnion();
  this->Internals = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkVgContourOperatorManager::~vtkVgContourOperatorManager()
{
  delete this->Internals;
  this->FilterBoolean->Delete();
  this->SelectorBoolean->Delete();
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::RemoveAllOperators()
{
  bool removeFilters = this->RemoveAllFilters();
  bool removeSelectors = this->RemoveAllSelectors();
  if (removeFilters || removeSelectors)
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::AddFilter(vtkPoints* loopPoints)
{
  if (this->Internals->AddContour(loopPoints, this->FilterBoolean,
                                  this->Internals->Filters, this->Internals->Selectors))
    {
    this->Modified();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::RemoveFilter(vtkPoints* loopPoints)
{
  if (this->Internals->RemoveContour(loopPoints, this->FilterBoolean,
                                     this->Internals->Filters))
    {
    this->Modified();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::RemoveAllFilters()
{
  if (this->Internals->RemoveAllContours(this->Internals->Filters,
                                         this->FilterBoolean))
    {
    this->Modified();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
int vtkVgContourOperatorManager::GetNumberOfFilters()
{
  return static_cast<int>(this->Internals->Filters.size());
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::AddSelector(vtkPoints* loopPoints)
{
  if (this->Internals->AddContour(loopPoints, this->SelectorBoolean,
                                  this->Internals->Selectors, this->Internals->Filters))
    {
    this->Modified();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::RemoveSelector(vtkPoints* loopPoints)
{
  if (this->Internals->RemoveContour(loopPoints, this->SelectorBoolean,
                                     this->Internals->Selectors))
    {
    this->Modified();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::RemoveAllSelectors()
{
  if (this->Internals->RemoveAllContours(this->Internals->Selectors,
                                         this->SelectorBoolean))
    {
    this->Modified();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
int vtkVgContourOperatorManager::GetNumberOfSelectors()
{
  return static_cast<int>(this->Internals->Selectors.size());
}

//-----------------------------------------------------------------------------
void vtkVgContourOperatorManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::EvaluatePath(vtkPoints* points, vtkIdList* ptIds)
{
  if (ptIds->GetNumberOfIds() == 0)
    {
    return true;
    }
  for (int i = 0; i < ptIds->GetNumberOfIds(); i++)
    {
    double* point = points->GetPoint(ptIds->GetId(i));
    if ((!this->GetNumberOfEnabledSelectors() ||
         this->SelectorBoolean->FunctionValue(point) < 0) &&
        (!this->GetNumberOfEnabledFilters() ||
         this->FilterBoolean->FunctionValue(point) > 0))
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::EvaluatePoint(double testPt[3])
{
  if ((!this->GetNumberOfEnabledSelectors() ||
       this->SelectorBoolean->FunctionValue(testPt) < 0) &&
      (!this->GetNumberOfEnabledFilters() ||
       this->FilterBoolean->FunctionValue(testPt) > 0))
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
unsigned long vtkVgContourOperatorManager::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();
  unsigned long time;

  // filters
  time = this->Internals->GetContoursMTime(this->Internals->Filters);
  mTime = time > mTime ? time : mTime;
  //selectors
  time = this->Internals->GetContoursMTime(this->Internals->Selectors);
  mTime = time > mTime ? time : mTime;

  return mTime;
}

//----------------------------------------------------------------------------
void vtkVgContourOperatorManager::SetContourEnabled(vtkPoints* loopPts, bool state)
{
  if (this->Internals->SetContourEnabled(loopPts, state,
                                         this->FilterBoolean, this->SelectorBoolean))
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
bool vtkVgContourOperatorManager::GetContourEnabled(vtkPoints* loopPts)
{
  return this->Internals->GetContourEnabled(loopPts);
}

//----------------------------------------------------------------------------
int vtkVgContourOperatorManager::GetNumberOfEnabledFilters()
{
  return this->Internals->GetNumberOfEnabledContours(this->Internals->Filters);
}

//----------------------------------------------------------------------------
int vtkVgContourOperatorManager::GetNumberOfEnabledSelectors()
{
  return this->Internals->GetNumberOfEnabledContours(this->Internals->Selectors);
}

//----------------------------------------------------------------------------
vtkImplicitSelectionLoop*
vtkVgContourOperatorManager::GetContourLoop(vtkPoints* loopPts)
{
  return this->Internals->GetContourLoop(loopPts);
}
