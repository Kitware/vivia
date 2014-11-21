/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgEventFilter.h"

#include "vtkVgEvent.h"

#include <vtkObjectFactory.h>

#include <map>

vtkStandardNewMacro(vtkVgEventFilter);

struct FilterInfo
{
  bool   Show;
  bool   Inverse;
  double MaxNormalcy;
  double MinProbability;

  FilterInfo()
    : Show(true), Inverse(false), MaxNormalcy(1.0), MinProbability(0.0) { }
};

//----------------------------------------------------------------------------
struct vtkVgEventFilter::vtkInternal
{
  std::map<int, FilterInfo> FilterInfoMap;
};

//-----------------------------------------------------------------------------
vtkVgEventFilter::vtkVgEventFilter()
{
  this->Internal = new vtkInternal;
}

//-----------------------------------------------------------------------------
vtkVgEventFilter::~vtkVgEventFilter()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgEventFilter::SetShowType(int eventType, bool state)
{
  this->Internal->FilterInfoMap[eventType].Show = state;
  this->Modified();
}

//-----------------------------------------------------------------------------
bool vtkVgEventFilter::GetShowType(int eventType)
{
  if (this->Internal->FilterInfoMap.find(eventType) ==
      this->Internal->FilterInfoMap.end())
    {
    return false;
    }
  return this->Internal->FilterInfoMap[eventType].Show;
}

//-----------------------------------------------------------------------------
void vtkVgEventFilter::SetInverse(int eventType, bool state)
{
  this->Internal->FilterInfoMap[eventType].Inverse = state;
  this->Modified();
}

//-----------------------------------------------------------------------------
bool vtkVgEventFilter::GetInverse(int eventType)
{
  if (this->Internal->FilterInfoMap.find(eventType) ==
      this->Internal->FilterInfoMap.end())
    {
    return false;
    }
  return this->Internal->FilterInfoMap[eventType].Inverse;
}

//-----------------------------------------------------------------------------
void vtkVgEventFilter::SetMaxNormalcy(int eventType, double threshold)
{
  this->Internal->FilterInfoMap[eventType].MaxNormalcy = threshold;
  this->Modified();
}

//-----------------------------------------------------------------------------
double vtkVgEventFilter::GetMaxNormalcy(int eventType)
{
  if (this->Internal->FilterInfoMap.find(eventType) ==
      this->Internal->FilterInfoMap.end())
    {
    return -1.0;
    }
  return this->Internal->FilterInfoMap[eventType].MaxNormalcy;
}

//-----------------------------------------------------------------------------
void vtkVgEventFilter::SetMinProbability(int eventType, double threshold)
{
  this->Internal->FilterInfoMap[eventType].MinProbability = threshold;
  this->Modified();
}

//-----------------------------------------------------------------------------
double vtkVgEventFilter::GetMinProbability(int eventType)
{
  if (this->Internal->FilterInfoMap.find(eventType) ==
      this->Internal->FilterInfoMap.end())
    {
    return 2.0;
    }
  return this->Internal->FilterInfoMap[eventType].MinProbability;
}

//-----------------------------------------------------------------------------
std::vector<vtkVgEventFilter::ScoredClassifier>
vtkVgEventFilter::GetActiveClassifiers(vtkVgEvent* event)
{
  std::vector<ScoredClassifier> classifiers;

  for (bool valid = event->InitClassifierTraversal(); valid;
       valid = event->NextClassifier())
    {
    int eventType = event->GetClassifierType();
    if (this->Internal->FilterInfoMap.find(eventType) ==
        this->Internal->FilterInfoMap.end())
      {
      continue;
      }
    const FilterInfo& info = this->Internal->FilterInfoMap[eventType];

    if (!info.Show)
      continue;

    if (info.Inverse)
      {
      if (event->GetClassifierNormalcy() < info.MaxNormalcy)
        continue;
      }
    else
      {
      if (event->GetClassifierNormalcy() > info.MaxNormalcy)
        continue;
      }

    if (info.Inverse)
      {
      if (event->GetClassifierProbability() > info.MinProbability)
        continue;
      }
    else
      {
      if (event->GetClassifierProbability() < info.MinProbability)
        continue;
      }

    double score = event->GetClassifierScore();
    classifiers.push_back(ScoredClassifier(eventType, score));
    }

  return classifiers;
}


//-----------------------------------------------------------------------------
int vtkVgEventFilter::GetBestClassifier(vtkVgEvent* event)
{
  int bestType = -1;
  double bestScore = -1.0;

  for (bool valid = event->InitClassifierTraversal(); valid;
       valid = event->NextClassifier())
    {
    int eventType = event->GetClassifierType();
    if (this->Internal->FilterInfoMap.find(eventType) ==
        this->Internal->FilterInfoMap.end())
      {
      continue;
      }
    const FilterInfo& info = this->Internal->FilterInfoMap[eventType];

    if (!info.Show)
      continue;

    if (info.Inverse)
      {
      if (event->GetClassifierNormalcy() < info.MaxNormalcy)
        continue;
      }
    else
      {
      if (event->GetClassifierNormalcy() > info.MaxNormalcy)
        continue;
      }

    if (info.Inverse)
      {
      if (event->GetClassifierProbability() > info.MinProbability)
        continue;
      }
    else
      {
      if (event->GetClassifierProbability() < info.MinProbability)
        continue;
      }

    double score = event->GetClassifierScore();
    if (score > bestScore)
      {
      bestScore = score;
      bestType = eventType;
      }
    }

  return bestType;
}

//-----------------------------------------------------------------------------
void vtkVgEventFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
