/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgTrack.h"

#include "vtkVgScalars.h"
#include "vtkVgTypeDefs.h"

// VTK includes.
#include <vtkActor.h>
#include <vtkBoundingBox.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkIdList.h>
#include <vtkSmartPointer.h>

#include <limits>
#include <map>
#include <numeric>

vtkStandardNewMacro(vtkVgTrack);
vtkCxxSetObjectMacro(vtkVgTrack, Points, vtkPoints);

typedef std::map<vtkVgTimeStamp, vtkIdType>         TimeToIdMap;
typedef TimeToIdMap::iterator                       TimeToIdMapIter;
typedef TimeToIdMap::const_iterator                 TimeToIdMapConstIter;
typedef std::pair<TimeToIdMapIter, TimeToIdMapIter> TimeToIdMapRange;

typedef vtkSmartPointer<vtkVgScalars> Scalars;

//----------------------------------------------------------------------------
class vtkVgTrack::vtkInternal
{
public:
  TimeToIdMap PointIdMap;     // no interpolated points
  TimeToIdMap AllPointsIdMap; // includes interpolated points
  TimeToIdMap HeadIdentifierStartIndex;

  vtkIdList* HeadIdentifierIds;
  TimeToIdMapConstIter AllPointsIdMapIterator;

  vtkTimeStamp BuildTime;
  bool LastDisplayedRegardlessOfFrame;
  bool ClosureOfEmptyTrack;

  std::map<std::string, Scalars> IdToScalarsMap;
  Scalars ActiveScalars;

  std::map<vtkVgTimeStamp, vtkVgGeoCoord> LatLons;

  vtkInternal()
    {
    this->HeadIdentifierIds = vtkIdList::New();
    // allocate memory for 10 rectangular track heads (each head is specified
    // via num points in head follow by the points, with 1st point repeated to
    // close the loop)
    this->HeadIdentifierIds->Allocate(60);
    this->LastDisplayedRegardlessOfFrame = false;
    this->ClosureOfEmptyTrack = false;
    this->ActiveScalars = 0;
    }

  ~vtkInternal()
    {
    this->HeadIdentifierIds->Delete();
    }

  // Sets the HeadIdentifier at the specified timeStamp.  Note, that
  // if this is called multiple times for the same timeStamp, memory
  // will be wasted in the HeadIdentifierIds.  However, choosing this
  // route because will result in less frequent memory allocation.
  // Not sure if this is the best route, but works for now...
  void SetHeadIdentifier(const vtkVgTimeStamp& timestamp,
                         vtkPoints* points, vtkIdType numberOfPts,
                         vtkPoints* fromPoints, vtkIdType fromPtsStart,
                         const float* pts = 0)
    {
    this->HeadIdentifierStartIndex[timestamp] =
      this->HeadIdentifierIds->GetNumberOfIds();
    this->HeadIdentifierIds->InsertNextId(
      numberOfPts > 1 ? numberOfPts + 1 : numberOfPts);

    // point ids must be consecutive
    if (fromPoints)
      {
      double point[3];
      for (vtkIdType i = 0; i < numberOfPts; i++)
        {
        fromPoints->GetPoint(fromPtsStart + i, point);
        this->HeadIdentifierIds->InsertNextId(points->InsertNextPoint(point));
        }
      }
    else
      {
      for (vtkIdType i = 0; i < numberOfPts; i++)
        {
        this->HeadIdentifierIds->InsertNextId(
          points->InsertNextPoint(pts + i * 3));
        }
      }

    // if greater than 1, then close the loop
    if (numberOfPts > 1)
      {
      this->HeadIdentifierIds->InsertNextId(
        this->HeadIdentifierIds->GetId(
          this->HeadIdentifierIds->GetNumberOfIds() - numberOfPts));
      }
    }

  void RemoveHeadIdentifier(const vtkVgTimeStamp& timeStamp)
    {
    this->HeadIdentifierStartIndex.erase(timeStamp);
    }

  // Get the half-open range in AllPointsIdMap containing all the interpolated
  // points which depend on the non-interpolated PointIdMap point given as the
  // parameter.
  TimeToIdMapRange GetInterpolatedPointsRange(TimeToIdMapConstIter pointIter)
    {
    TimeToIdMapRange result;

    // find the first ipoint with timestamp> previous non-interpolated point
    if (pointIter != this->PointIdMap.begin())
      {
      TimeToIdMapConstIter beforeIter = pointIter;
      --beforeIter;
      result.first = this->AllPointsIdMap.find(beforeIter->first);
      ++result.first;
      }
    else
      {
      result.first = this->AllPointsIdMap.begin();
      }

    // find the first ipoint with timestamp == the next non-interpolated point
    if (pointIter != --this->PointIdMap.end())
      {
      TimeToIdMapConstIter afterIter = pointIter;
      ++afterIter;
      result.second = this->AllPointsIdMap.find(afterIter->first);
      }
    else
      {
      result.second = this->AllPointsIdMap.end();
      }

    return result;
    }

  // Get the half-open range in AllPointsIdMap containing all the interpolated
  // heads which depend on the head corresponding to the non-interpolated
  // PointIdMap point given as the parameter. The returned range will be empty
  // if there is no head with this point's timestamp. If a non-interpolated
  // head does exist, it is included in the range as well (by necessity).
  TimeToIdMapRange GetInterpolatedHeadsRange(TimeToIdMapConstIter pointIter)
    {
    TimeToIdMapRange result;

    // find the first head with timestamp> previous non-interpolated point
    if (pointIter != this->PointIdMap.begin())
      {
      TimeToIdMapConstIter beforeIter = pointIter;
      --beforeIter;
      result.first
        = this->HeadIdentifierStartIndex.upper_bound(beforeIter->first);
      }
    else
      {
      result.first = this->HeadIdentifierStartIndex.begin();
      }

    // find the first head with timestamp>= the next non-interpolated point
    if (pointIter != --this->PointIdMap.end())
      {
      TimeToIdMapConstIter afterIter = pointIter;
      ++afterIter;
      result.second
        = this->HeadIdentifierStartIndex.lower_bound(afterIter->first);
      }
    else
      {
      result.second = this->HeadIdentifierStartIndex.end();
      }

    return result;
    }

  // Get iterator to entry in map closest to timeStamp. If the map is empty,
  // return end().
  TimeToIdMapConstIter GetClosestMapEntry(TimeToIdMap& theMap,
                                          const vtkVgTimeStamp& timeStamp)
    {
    if (theMap.empty())
      {
      return theMap.end();
      }

    TimeToIdMapIter candidateAfter = theMap.lower_bound(timeStamp);

    // Did we find an exact match?
    if (candidateAfter != theMap.end() && candidateAfter->first == timeStamp)
      {
      return candidateAfter;
      }

    // As long as we're not at the beginning of the map, we know we can step
    // back one entry and have a valid iterator.
    TimeToIdMapIter candidateBefore = theMap.end();
    if (candidateAfter != theMap.begin())
      {
      candidateBefore = candidateAfter;
      candidateBefore--;
      }

    // Return the "candidateAfter" if we don't have a candidate before (since
    // the map is not empty, must therefore have one after) or we have
    // candidates both before and after and the candidate after is closer.
    if (candidateBefore == theMap.end() ||
        (candidateAfter != theMap.end() &&
         candidateAfter->first.GetTimeDifferenceInSecs(timeStamp) <
         timeStamp.GetTimeDifferenceInSecs(candidateBefore->first)))
      {
      return candidateAfter;
      }

    // Either the candidate before is closer, or there was no candidate after.
    return candidateBefore;
    }
};

//-----------------------------------------------------------------------------
vtkVgTrack::vtkVgTrack()
{
  this->Type = -1;
  this->Id = 0;
  this->Flags = 0;
  this->DisplayFlags = DF_Normal;
  this->InterpolateMissingPointsOnInsert = false;
  this->InterpolateToGround = false;

  this->InterpolationSpacing.SetFrameNumber(1);
  this->InterpolationSpacing.SetTime(0.5e6);

  this->Name = 0;
  this->Note = 0;

  this->Normalcy = 1.0;

  // track representation
  this->Points = 0;
  this->PointIds = vtkIdList::New();

  // default to unclassified...
  this->PVO[0] = this->PVO[1] = this->PVO[2] = 0.0;

  this->Color[0] = 1.0;
  this->Color[1] = 1.0;
  this->Color[2] = 0.0;

  this->UseCustomColor = false;
  this->CustomColor[0] = this->CustomColor[1] = this->CustomColor[2] = 0.5;

  this->Status = vgObjectStatus::None;

  this->Internal = new vtkInternal;
}

//-----------------------------------------------------------------------------
vtkVgTrack::~vtkVgTrack()
{
  delete [] this->Name;
  delete [] this->Note;
  if (this->Points)
    {
    this->Points->Delete();
    }
  this->PointIds->Delete();
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::Allocate(vtkIdType numberOfFrames)
{
  this->PointIds->Allocate(numberOfFrames);
}

//-----------------------------------------------------------------------------
void vtkVgTrack::CopyData(vtkVgTrack* other)
{
  this->Internal->PointIdMap = other->Internal->PointIdMap;
  this->Internal->AllPointsIdMap = other->Internal->AllPointsIdMap;
  this->Internal->HeadIdentifierStartIndex =
    other->Internal->HeadIdentifierStartIndex;

  this->Internal->HeadIdentifierIds->DeepCopy(
    other->Internal->HeadIdentifierIds);

  this->SetPoints(other->GetPoints());
  this->PointIds->DeepCopy(other->GetPointIds());

  this->StartFrame = other->StartFrame;
  this->EndFrame = other->EndFrame;

  this->Normalcy = other->Normalcy;

  this->PVO[0] = other->PVO[0];
  this->PVO[1] = other->PVO[1];
  this->PVO[2] = other->PVO[2];

  this->TOC = other->TOC;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::Merge(vtkVgTrack* other)
{
  TimeToIdMapConstIter headIter =
    other->Internal->HeadIdentifierStartIndex.begin();

  vtkVgTimeStamp origStart = this->GetStartFrame();
  vtkVgTimeStamp origEnd = this->GetEndFrame();

  for (TimeToIdMapConstIter iter = other->Internal->PointIdMap.begin(),
                             end = other->Internal->PointIdMap.end();
       iter != end; ++iter)
    {
    if (iter->first >= origStart && iter->first <= origEnd)
      {
      continue;
      }

    double point[3];
    other->Points->GetPoint(iter->second, point);

    vtkVgGeoCoord geoCoord = other->GetGeoCoord(iter->first);

    while (headIter->first < iter->first)
      {
      ++headIter;
      }

    vtkIdType npts = 0;
    vtkIdType firstPtId = -1;
    if (headIter->first == iter->first)
      {
      npts = other->Internal->HeadIdentifierIds->GetId(headIter->second) - 1;
      firstPtId = other->Internal->HeadIdentifierIds->GetId(headIter->second + 1);
      }

    this->SetPoint(iter->first, point, geoCoord, npts, 0,
                   other->Points, firstPtId);
    }
}

//-----------------------------------------------------------------------------
double* vtkVgTrack::GetFullBounds()
{
  if (!this->Points)
    {
    vtkErrorMacro("Points must be set!");
    return 0;
    }

  vtkBoundingBox bbox;
  vtkIdList* pointIds = this->GetPointIds();
  for (vtkIdType i = 0; i < pointIds->GetNumberOfIds(); i++)
    {
    double* pt = this->Points->GetPoint(pointIds->GetId(i));
    bbox.AddPoint(pt);
    }
  bbox.GetBounds(this->FullBounds);
  return this->FullBounds;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::GetFullBounds(double bounds[4])
{
  this->GetFullBounds();
  bounds[0] = this->FullBounds[0];
  bounds[1] = this->FullBounds[1];
  bounds[2] = this->FullBounds[2];
  bounds[3] = this->FullBounds[3];
}

//-----------------------------------------------------------------------------
void vtkVgTrack::InsertNextPoint(const vtkVgTimeStamp& timeStamp,
                                 const double point[2],
                                 const vtkVgGeoCoord& geoCoord,
                                 vtkIdType numberOfShellPts,
                                 const float* shellPts,
                                 bool interpolateShell,
                                 vtkPoints* fromShellPoints,
                                 vtkIdType fromShellPtsStart)
{
  if (!this->Points)
    {
    vtkErrorMacro("Must set Points before inserting track points!");
    return;
    }
  if (this->EndFrame.IsValid())
    {
    vtkErrorMacro("The track has been Closed... can't add addition points!");
    return;
    }
  if (this->Internal->PointIdMap.size() > 0 &&
      timeStamp <= this->Internal->PointIdMap.rbegin()->first)
    {
    vtkErrorMacro("Can ONLY add point to track after ALL previously added points!");
    return;
    }

  vtkIdType ptId;
  if (this->Internal->PointIdMap.size() == 0)
    {
    this->StartFrame = timeStamp;
    if (this->Internal->ClosureOfEmptyTrack)
      {
      this->EndFrame = timeStamp;
      }
    }
  else if (this->InterpolateMissingPointsOnInsert)
    {
    const vtkVgTimeStamp& prevTimeStamp
      = this->Internal->PointIdMap.rbegin()->first;

    double prevPt[3];
    this->Points->GetPoint(this->Internal->PointIdMap.rbegin()->second,
                           prevPt);

    this->AddInterpolationPoints(prevTimeStamp, timeStamp, prevPt, point,
                                 interpolateShell ? numberOfShellPts : 0,
                                 fromShellPoints, fromShellPtsStart, shellPts);
    }
  ptId = this->Points->InsertNextPoint(point[0], point[1], 0.0);
  this->Internal->PointIdMap[timeStamp] = ptId;
  this->Internal->AllPointsIdMap[timeStamp] = ptId;

  if (geoCoord.IsValid())
    {
    this->Internal->LatLons[timeStamp] = geoCoord;
    }

  if (numberOfShellPts > 0)
    {
    this->Internal->SetHeadIdentifier(timeStamp, this->Points,
                                      numberOfShellPts, fromShellPoints,
                                      fromShellPtsStart, shellPts);
    }

  this->PointIds->Reset();
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrack::InsertNextPoint(
  const vtkVgTimeStamp& timeStamp, const double point[2],
  const vtkVgGeoCoord& geoCoord,
  vtkDenseArray<double>* shellPts)
{
  const vtkArray::SizeT shellArraySize = (shellPts ? shellPts->GetSize() : 0);
  if (shellArraySize && ((shellArraySize % 3) == 0))
    {
    // Since python does not support single precision floats, we must convert
    // the shell points array instead of just using vtkDenseArray::GetStorage().
    float* shellPtsF = new float[shellArraySize];
    for(vtkArray::SizeT i = 0; i < shellArraySize; i++)
      {
      shellPtsF[i] = static_cast<float>(shellPts->GetValueN(i));
      }
    this->InsertNextPoint(timeStamp, point, geoCoord,
                          shellArraySize / 3, shellPtsF);
    delete[] shellPtsF;
    }
  else
    {
    this->InsertNextPoint(timeStamp, point, geoCoord, 0, 0);
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrack::AddInterpolationPoints(const vtkVgTimeStamp& previousTimeStamp,
                                        const vtkVgTimeStamp& timeStamp,
                                        const double previousPoint[2],
                                        const double point[2],
                                        vtkIdType numShellPts,
                                        vtkPoints* fromShellPts,
                                        vtkIdType fromShellPtsStart,
                                        const float* shellPts,
                                        bool warnOnFailure)
{
  vtkIdType numFrames;
  if (timeStamp.HasTime())
    {
    double timeDelta = timeStamp.GetTime() - previousTimeStamp.GetTime();
    numFrames = static_cast<vtkIdType>(
                  0.5 + timeDelta / this->InterpolationSpacing.GetTime());
    }
  else // timeStamp has frame #
    {
    double frameDelta = timeStamp.GetFrameNumber() -
                        previousTimeStamp.GetFrameNumber();
    numFrames = static_cast<vtkIdType>(
                  0.5 + frameDelta / this->InterpolationSpacing.GetFrameNumber());
    }

  if (numFrames <= 1)
    {
    return;
    }

  vtkIdType numPreviousShellPts, *prevShellPtIds, pointId;
  this->GetHeadIdentifier(previousTimeStamp, numPreviousShellPts,
                          prevShellPtIds, pointId);
  // If multiple points in the head, the version pulled by GetHeadIdentifier
  // has the first point duplicated at the end (we do not want to consider that
  // point when doing the interpolation.
  if (numPreviousShellPts > 1)
    {
    --numPreviousShellPts;
    }

  // get the start index, since the pointer may be invalidated once we start
  // allocating new heads
  vtkIdType headIdsStart
    = prevShellPtIds - this->Internal->HeadIdentifierIds->GetPointer(0);

  vtkVgTimeStamp interpolateTimeStamp = previousTimeStamp;
  interpolateTimeStamp.ShiftForward(this->InterpolationSpacing);

  if (!this->InterpolateToGround)
    {
    float* shellPtsTmp = 0;
    double delta[2] =
      {
      (point[0] - previousPoint[0]) / numFrames,
      (point[1] - previousPoint[1]) / numFrames
      };
    if (numShellPts != 0 && numPreviousShellPts != 0)
      {
      if (numShellPts != numPreviousShellPts)
        {
        if (warnOnFailure)
          {
          vtkErrorMacro("Track regions do not have the same number of points - "
            "interpolating centers only.");
          }
        }
      else
        {
        // allocate scratch space for doing interpolation of head regions,
        // which could have an unknown number of points
        shellPtsTmp = new float[3 * numShellPts];
        }
      }

    vtkIdType ptId;
    for (int j = 1; j < numFrames; j++)
      {
      ptId = this->Points->InsertNextPoint(previousPoint[0] + delta[0] * j,
        previousPoint[1] + delta[1] * j,
        0.0);

      this->Internal->AllPointsIdMap[interpolateTimeStamp] = ptId;

      if (shellPtsTmp)
        {
        // compute interpolated region for this frame
        float tempPt[3];
        for (vtkIdType p = 0; p < numShellPts; ++p)
          {
          const float* shellPt;
          if (fromShellPts)
            {
            double* pt = fromShellPts->GetPoint(fromShellPtsStart + p);
            tempPt[0] = pt[0];
            tempPt[1] = pt[1];
            tempPt[2] = pt[2];
            shellPt = tempPt;
            }
          else
            {
            shellPt = shellPts + p * 3;
            }

          double prevShellPt[3];
          this->Points->GetPoint(
            this->Internal->HeadIdentifierIds->GetId(headIdsStart + p),
            prevShellPt);

          double delta[3] =
            {
            j* ((shellPt[0] - prevShellPt[0]) / numFrames),
            j* ((shellPt[1] - prevShellPt[1]) / numFrames),
            j* ((shellPt[2] - prevShellPt[2]) / numFrames)
            };

          shellPtsTmp[p * 3 + 0] = prevShellPt[0] + delta[0];
          shellPtsTmp[p * 3 + 1] = prevShellPt[1] + delta[1];
          shellPtsTmp[p * 3 + 2] = prevShellPt[2] + delta[2];
          }

        // insert interpolated head region
        this->Internal->SetHeadIdentifier(interpolateTimeStamp, this->Points,
          numShellPts, 0, -1, shellPtsTmp);
        }

      // advance to next "interpolated" timestamp
      interpolateTimeStamp.ShiftForward(this->InterpolationSpacing);
      }

    delete[] shellPtsTmp;
    }
  else // Homography based "stabilization" for interpolation
    {
    int maxShellPoints =
      (numPreviousShellPts > numShellPts) ? numPreviousShellPts
                                          : numShellPts;
    float* shellPtsTmp = new float[3 * maxShellPoints];

    int midFrame;
    if (numFrames % 2)
      {
      midFrame = (numFrames + 1) / 2;
      }
    else
      {
      // TODO:
      // if an even number of segments (numFrames), which means an add number of
      // inserted points, figure out which non-interpolated point the middle
      // interpolated point is closer to.
      midFrame = (numFrames) / 2;
      }
    int j;
    // Stabilize the first chunk of frames based on the previous keyframe
    for (j = 1; j < midFrame; j++)
      {
      vtkIdType ptId = this->Points->InsertNextPoint(previousPoint[0], previousPoint[1], 0.0);
      this->Internal->AllPointsIdMap[interpolateTimeStamp] = ptId;

      // compute interpolated region for this frame
      for (vtkIdType p = 0; p < numPreviousShellPts; ++p)
        {
        double prevShellPt[3];
        this->Points->GetPoint(
          this->Internal->HeadIdentifierIds->GetId(headIdsStart + p),
          prevShellPt);

        shellPtsTmp[p * 3 + 0] = prevShellPt[0];
        shellPtsTmp[p * 3 + 1] = prevShellPt[1];
        shellPtsTmp[p * 3 + 2] = prevShellPt[2];
        }

      // insert interpolated head region
      this->Internal->SetHeadIdentifier(interpolateTimeStamp, this->Points,
        numPreviousShellPts, 0, -1, shellPtsTmp);

      // advance to next "interpolated" timestamp
      interpolateTimeStamp.ShiftForward(this->InterpolationSpacing);
      }
    // And the second chunk based on the next keyframe
    for (; j < numFrames; j++)
      {
      vtkIdType ptId = this->Points->InsertNextPoint(point[0], point[1], 0.0);
      this->Internal->AllPointsIdMap[interpolateTimeStamp] = ptId;

      float tempPt[3];
      for (vtkIdType p = 0; p < numShellPts; ++p)
        {
        const float* shellPt;
        if (fromShellPts)
          {
          double* pt = fromShellPts->GetPoint(fromShellPtsStart + p);
          tempPt[0] = pt[0];
          tempPt[1] = pt[1];
          tempPt[2] = pt[2];
          shellPt = tempPt;
          }
        else
          {
          shellPt = shellPts + p * 3;
          }

        shellPtsTmp[p * 3 + 0] = shellPt[0];
        shellPtsTmp[p * 3 + 1] = shellPt[1];
        shellPtsTmp[p * 3 + 2] = shellPt[2];
        }

      // insert interpolated head region
      this->Internal->SetHeadIdentifier(interpolateTimeStamp, this->Points,
        numShellPts, 0, -1, shellPtsTmp);

      // advance to next "interpolated" timestamp
      interpolateTimeStamp.ShiftForward(this->InterpolationSpacing);
      }
    delete[] shellPtsTmp;
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrack::SetPoint(const vtkVgTimeStamp& timeStamp,
                          const double point[2],
                          vtkVgGeoCoord geoCoord,
                          vtkIdType numberOfShellPts, const float* shellPts,
                          vtkPoints* fromShellPts,
                          vtkIdType fromShellPtsStart)
{
  // if not closed and after the last point, just use InsertNextPoint()
  if (!this->EndFrame.IsValid() &&
      (this->Internal->PointIdMap.size() == 0 ||
       timeStamp > this->Internal->PointIdMap.rbegin()->first))
    {
    this->InsertNextPoint(timeStamp, point, geoCoord,
                          numberOfShellPts, shellPts, true,
                          fromShellPts, fromShellPtsStart);
    return;
    }

  if (!this->Points)
    {
    vtkErrorMacro("Must set Points before inserting track points!");
    return;
    }

  // any restrictions whatsoever on the timeStamp.... can it be before
  // start or after end (even if closed)?

  // could similarly verify we aren't repeating head/shell points
  if (numberOfShellPts > 0)
    {
    this->Internal->SetHeadIdentifier(timeStamp, this->Points,
                                      numberOfShellPts, fromShellPts,
                                      fromShellPtsStart, shellPts);
    }
  else
    {
    // just in case we previously had a head identifier at this timeStamp
    this->Internal->RemoveHeadIdentifier(timeStamp);
    }

  vtkIdType ptId = this->Points->InsertNextPoint(point[0], point[1], 0.0);
  this->Internal->PointIdMap[timeStamp] = ptId;
  this->BuildAllPointsIdMap(timeStamp, ptId, point);
  this->PointIds->Reset();

  if (geoCoord.IsValid())
    {
    this->Internal->LatLons[timeStamp] = geoCoord;
    }
  else
    {
    this->Internal->LatLons.erase(timeStamp);
    }

  // make sure StartFrame and EndFrame (if track has been closed) are valid
  if (this->EndFrame.IsValid() && timeStamp > this->EndFrame)
    {
    this->EndFrame = timeStamp;
    }
  else if (timeStamp < this->StartFrame)
    {
    this->StartFrame = timeStamp;
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrack::DeletePoint(const vtkVgTimeStamp& timeStamp,
                             bool isBatchDelete)
{
  // does it even exist (need to know if we need to rebuild AllPointIdsMap)
  TimeToIdMapIter removeIter = this->Internal->PointIdMap.find(timeStamp);
  if (removeIter == this->Internal->PointIdMap.end())
    {
    return;  // no track point for the timeStamp, so nothing to do
    }

  if (this->Internal->PointIdMap.size() == 1)
    {
    vtkErrorMacro("Can't delete the only point in a track!");
    return;
    }

  // get points before and after the point being deleted (if exist)
  TimeToIdMapRange iPointsRange
    = this->Internal->GetInterpolatedPointsRange(removeIter);

  TimeToIdMapRange iHeadsRange
    = this->Internal->GetInterpolatedHeadsRange(removeIter);

  // erase interpolated points and (interp and non-interp) heads
  this->Internal->AllPointsIdMap.erase(iPointsRange.first,
                                       iPointsRange.second);

  this->Internal->HeadIdentifierStartIndex.erase(iHeadsRange.first,
                                                 iHeadsRange.second);

  // if there are track points before AND after the one we removed,
  // need to interpolate between them
  if (this->InterpolateMissingPointsOnInsert &&
      removeIter != this->Internal->PointIdMap.begin() &&
      removeIter != --this->Internal->PointIdMap.end())
    {
    TimeToIdMapConstIter pointBeforeIter(removeIter);
    TimeToIdMapConstIter pointAfterIter(removeIter);
    --pointBeforeIter;
    ++pointAfterIter;

    double pointBefore[3];
    this->Points->GetPoint(pointBeforeIter->second, pointBefore);

    vtkIdType npts, *pts, pointId;
    this->GetHeadIdentifier(pointAfterIter->first, npts, pts, pointId);

    double pointAfter[3];
    this->Points->GetPoint(pointAfterIter->second, pointAfter);

    this->AddInterpolationPoints(pointBeforeIter->first, pointAfterIter->first,
                                 pointBefore, pointAfter,
                                 npts > 1 ? npts - 1 : npts,
                                 this->Points, npts == 0 ? -1 : pts[0], 0,
                                 !isBatchDelete);
    }

  // erase the deleted point
  this->Internal->PointIdMap.erase(removeIter);
  this->PointIds->Reset();

  this->Internal->LatLons.erase(timeStamp);

  // make sure StartFrame and EndFrame (if track has been closed) are valid
  if (this->EndFrame.IsValid() && timeStamp == this->EndFrame)
    {
    this->EndFrame = this->Internal->PointIdMap.rbegin()->first;
    }
  else if (timeStamp == this->StartFrame)
    {
    this->StartFrame = this->Internal->PointIdMap.begin()->first;
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrack::BuildAllPointsIdMap(const vtkVgTimeStamp& timeStamp,
                                     vtkIdType newTrackPtId,
                                     const double point[2])
{
  // current assumption is that this method is called only from SetPoint, and
  // every time a new point is added (and not simple case where we can call
  // InsertNextPoint).  I can imagine that we might instead call
  // only when AllPointsIdMap is actually needed, in which case some of the
  // logic would need to be changed (since multiple points might have been
  // added since the last time it was called).

  // if no interpolation points, just copy PointIdMap
  if (!this->InterpolateMissingPointsOnInsert)
    {
    this->Internal->AllPointsIdMap = this->Internal->PointIdMap;
    return;
    }

  // given assumption stated above, we must have just inserted a point at a
  // timestamp before the latest timestamp;  only want to recompute
  // interpolation points around the modified point.  Thus, erase all map
  // entries around the newly inserted (or modified) point, then...

  // get iterator to the point we just set in SetPoint
  TimeToIdMapConstIter currentPointIdIter =
    this->Internal->PointIdMap.find(timeStamp);

  TimeToIdMapRange iPointsRange
    = this->Internal->GetInterpolatedPointsRange(currentPointIdIter);

  TimeToIdMapRange iHeadsRange
    = this->Internal->GetInterpolatedHeadsRange(currentPointIdIter);

  // erase interpolated points
  this->Internal->AllPointsIdMap.erase(iPointsRange.first,
                                       iPointsRange.second);

  // erase interpolated heads
  TimeToIdMapIter head
    = this->Internal->HeadIdentifierStartIndex.find(timeStamp);
  if (head != this->Internal->HeadIdentifierStartIndex.end())
    {
    // skip over the non-interpolated head at this timestamp
    this->Internal->HeadIdentifierStartIndex.erase(iHeadsRange.first,
                                                   head);
    this->Internal->HeadIdentifierStartIndex.erase(++head,
                                                   iHeadsRange.second);
    }
  else
    {
    this->Internal->HeadIdentifierStartIndex.erase(iHeadsRange.first,
                                                   iHeadsRange.second);
    }

  // interpolate before new point
  if (currentPointIdIter != this->Internal->PointIdMap.begin())
    {
    TimeToIdMapConstIter pointBeforeIter(currentPointIdIter);
    --pointBeforeIter;

    vtkIdType npts, *ptIds, pointId;
    this->GetHeadIdentifier(timeStamp, npts, ptIds, pointId);

    double pointBefore[3];
    this->Points->GetPoint(pointBeforeIter->second, pointBefore);

    this->AddInterpolationPoints(pointBeforeIter->first, timeStamp,
                                 pointBefore, point,
                                 npts > 1 ? npts - 1 : npts,
                                 this->Points, npts == 0 ? -1 : ptIds[0]);
    }

  // set the new / modified point
  this->Internal->AllPointsIdMap[timeStamp] = newTrackPtId;

  // interpolate after new point
  if (currentPointIdIter != --this->Internal->PointIdMap.end())
    {
    TimeToIdMapConstIter pointAfterIter(currentPointIdIter);
    ++pointAfterIter;

    vtkIdType npts, *ptIds, pointId;
    this->GetHeadIdentifier(pointAfterIter->first, npts, ptIds, pointId);

    double pointAfter[3];
    this->Points->GetPoint(pointAfterIter->second, pointAfter);

    this->AddInterpolationPoints(timeStamp, pointAfterIter->first,
                                 point, pointAfter,
                                 npts > 1 ? npts - 1 : npts,
                                 this->Points, npts == 0 ? -1 : ptIds[0]);
    }
}

//-----------------------------------------------------------------------------
void vtkVgTrack::Close()
{
  if (this->Internal->PointIdMap.size() == 0)
    {
    // Closing an empty track; set flag indicating as much and we'll set the
    // EndFrame (the main indicator of whether a track has been closed) when
    // the 1st point is added to the track
    this->Internal->ClosureOfEmptyTrack = true;
    return;
    }
  this->EndFrame = this->Internal->PointIdMap.rbegin()->first;
}

//-----------------------------------------------------------------------------
vtkIdList* vtkVgTrack::GetPointIds()
{
  // build the list if it doesn't equal the number of entries in our map
  // NOTE: temporary variables are to avoid signed/unsigned mismatch
  unsigned long long listSize = this->PointIds->GetNumberOfIds();
  unsigned long long mapSize = this->Internal->AllPointsIdMap.size();
  if (listSize != mapSize)
    {
    this->PointIds->Allocate(
      static_cast<vtkIdType>(this->Internal->AllPointsIdMap.size()));

    TimeToIdMapConstIter pointIdIter;
    for (pointIdIter = this->Internal->AllPointsIdMap.begin();
         pointIdIter != this->Internal->AllPointsIdMap.end(); pointIdIter++)
      {
      this->PointIds->InsertNextId(pointIdIter->second);
      }
    }
  return this->PointIds;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::GetHeadIdentifier(const vtkVgTimeStamp& timeStamp,
                                   vtkIdType& npts, vtkIdType*& pts,
                                   vtkIdType& trackPointId,
                                   double tolerance/*= 0.001*/) const
{
  trackPointId = -1; // "invalid" value unless we set it

  TimeToIdMapConstIter headIter = this->Internal->GetClosestMapEntry(
                                    this->Internal->HeadIdentifierStartIndex, timeStamp);
  if (headIter != this->Internal->HeadIdentifierStartIndex.end() &&
      fabs(headIter->first.GetTimeDifferenceInSecs(timeStamp)) < tolerance)
    {
    npts = this->Internal->HeadIdentifierIds->GetId(headIter->second);
    pts = this->Internal->HeadIdentifierIds->GetPointer(headIter->second + 1);
    return;
    }

  // No matching head identifier; make sure the result values are zeroed out.
  npts = 0;
  pts = 0;

  // No head identifier; check if there is a track point within tolerance.
  headIter = this->Internal->GetClosestMapEntry(this->Internal->AllPointsIdMap,
                                                timeStamp);
  if (headIter != this->Internal->AllPointsIdMap.end() &&
      fabs(headIter->first.GetTimeDifferenceInSecs(timeStamp)) < tolerance)
    {
    trackPointId = headIter->second;
    }
}

//-----------------------------------------------------------------------------
vtkBoundingBox vtkVgTrack::GetHeadBoundingBox(const vtkVgTimeStamp& timeStamp,
                                              double tolerance) const
{
  vtkIdType npts, *ptIds, ptId;
  this->GetHeadIdentifier(timeStamp, npts, ptIds, ptId, tolerance);

  vtkBoundingBox bbox;
  for (vtkIdType i = 0; i < npts; ++i)
    {
    bbox.AddPoint(this->Points->GetPoint(ptIds[i]));
    }

  return bbox;
}

//-----------------------------------------------------------------------------
vtkVgGeoCoord vtkVgTrack::GetGeoCoord(const vtkVgTimeStamp& timeStamp)
{
  std::map<vtkVgTimeStamp, vtkVgGeoCoord>::iterator itr =
    this->Internal->LatLons.find(timeStamp);
  if (itr != this->Internal->LatLons.end())
    {
    return itr->second;
    }
  return vtkVgGeoCoord();
}

//-----------------------------------------------------------------------------
bool vtkVgTrack::GetFrameIsInterpolated(const vtkVgTimeStamp& timeStamp)
{
  return this->Internal->PointIdMap.find(timeStamp) ==
         this->Internal->PointIdMap.end();
}

//-----------------------------------------------------------------------------
vtkVgTrackDisplayData vtkVgTrack::GetDisplayData(vtkVgTimeStamp start,
                                                 vtkVgTimeStamp end)
{
  if (!this->StartFrame.IsValid())
    {
    return vtkVgTrackDisplayData(); // track not started
    }

  vtkIdList* ids = this->GetPointIds();

  vtkIdType startIndex;
  if (start.IsMinTime())
    {
    startIndex = 0;
    }
  else
    {
    vtkIdType startId = this->GetClosestFramePtId(start);
    startIndex = ids->IsId(startId); // linear lookup
    }

  vtkIdType endIndex;
  if (end.IsMaxTime())
    {
    endIndex = ids->GetNumberOfIds() - 1;
    }
  else
    {
    vtkIdType endId = this->GetClosestFramePtId(end);
    endIndex = ids->IsId(endId); // linear lookup
    }

  vtkVgTrackDisplayData tdi;
  tdi.IdsStart = ids->GetPointer(startIndex);
  tdi.NumIds = endIndex - startIndex + 1;

  if (!this->GetActiveScalars())
    {
    return tdi;
    }

  TimeToIdMap::const_iterator itr =
    this->Internal->AllPointsIdMap.upper_bound(start);

  // Follow the same logic as implemented in GetClosestFramePtId(..)
  if( (itr != this->Internal->AllPointsIdMap.begin()) &&
       itr != this->Internal->AllPointsIdMap.end())
    {
    --itr;
    }

  vtkIdType count = 0;
  for(; (count < tdi.NumIds) && itr != this->Internal->AllPointsIdMap.end();
      ++count, ++itr)
    {
    tdi.Scalars.push_back(
      this->Internal->ActiveScalars->GetValue(itr->first, true));
    }

  assert(tdi.Scalars.size() == static_cast<size_t>(tdi.NumIds));

  return tdi;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::SetInterpolationSpacing(const vtkVgTimeStamp& timeStamp)
{
  if (this->InterpolationSpacing == timeStamp)
    {
    return;
    }

  this->InterpolationSpacing = timeStamp;
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vtkVgTrack::GetEndFrame() const
{
  vtkVgTimeStamp endFrame;
  if (this->EndFrame.IsValid())
    {
    return this->EndFrame;
    }
  else if (this->StartFrame.IsValid())
    {
    return this->Internal->PointIdMap.rbegin()->first;
    }
  else
    {
    vtkGenericWarningMacro("Track hasn't been started!");
    }

  return endFrame;
}

//-----------------------------------------------------------------------------
bool vtkVgTrack::GetPoint(const vtkVgTimeStamp& timeStamp, double pt[2],
                          bool includeInterpolated)
{
  TimeToIdMapConstIter iter;
  if (includeInterpolated)
    {
    iter = this->Internal->AllPointsIdMap.find(timeStamp);
    if (iter == this->Internal->AllPointsIdMap.end())
      {
      return false;
      }
    }
  else
    {
    iter = this->Internal->PointIdMap.find(timeStamp);
    if (iter == this->Internal->PointIdMap.end())
      {
      return false;
      }
    }

  double point[3];
  this->Points->GetPoint(iter->second, point);
  pt[0] = point[0];
  pt[1] = point[1];
  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgTrack::GetClosestFramePt(const vtkVgTimeStamp& timeStamp,
                                   double pt[2])
{
  vtkIdType ptId = this->GetClosestFramePtId(timeStamp);

  if (ptId != -1)
    {
    double point3D[3];
    this->Points->GetPoint(ptId, point3D);

    pt[0] = point3D[0];
    pt[1] = point3D[1];
    return true;
    }
  return false;
}


//-----------------------------------------------------------------------------
vtkIdType vtkVgTrack::GetClosestFramePtId(const vtkVgTimeStamp& timeStamp)
{
  if (!this->StartFrame.IsValid()) // track not started
    {
    vtkErrorMacro("Trying to retrieve ClosestFramePtId from track that hasn't been started!");
    return -1;
    }

  TimeToIdMapConstIter ptIdIter;
  ptIdIter = this->Internal->AllPointsIdMap.lower_bound(timeStamp);
  if (ptIdIter == this->Internal->AllPointsIdMap.end() ||
      (ptIdIter != this->Internal->AllPointsIdMap.begin() &&
       timeStamp < ptIdIter->first))
    {
    ptIdIter--;
    }

  return ptIdIter->second;
}

//-----------------------------------------------------------------------------
bool vtkVgTrack::GetPriorFramePt(const vtkVgTimeStamp& timeStamp, double pt[2],
                                 vtkVgTimeStamp& priorTimeStamp)
{
  vtkIdType ptId = this->GetPriorFramePtId(timeStamp, priorTimeStamp);

  if (ptId == -1)
    {
    return false;
    }

  double point3D[3];
  this->Points->GetPoint(ptId, point3D);

  pt[0] = point3D[0];
  pt[1] = point3D[1];
  return true;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrack::GetPriorFramePtId(const vtkVgTimeStamp& timeStamp,
                                        vtkVgTimeStamp& priorTimeStamp)
{
  if (!this->StartFrame.IsValid())   // track not started
    {
    vtkErrorMacro("Trying to retrieve ClosestFramePtId from track that hasn't been started!");
    return -1;
    }

  TimeToIdMapConstIter ptIdIter;
  ptIdIter = this->Internal->AllPointsIdMap.lower_bound(timeStamp);
  if (ptIdIter == this->Internal->AllPointsIdMap.begin())
    {
    // we're interested in the item before lower_bound, so if we're at the
    // beginning of the map we're not going to find what we want.
    return -1;
    }

  ptIdIter--;
  priorTimeStamp = ptIdIter->first;
  return ptIdIter->second;
}

//-----------------------------------------------------------------------------
bool vtkVgTrack::GetFrameAtOrBefore(vtkVgTimeStamp& timeStamp)
{
  if (this->Internal->PointIdMap.empty())
    {
    return false;
    }

  TimeToIdMapConstIter ptIdIter;
  ptIdIter = this->Internal->PointIdMap.lower_bound(timeStamp);

  if (ptIdIter != this->Internal->PointIdMap.end() &&
      ptIdIter->first == timeStamp)
    {
    timeStamp = ptIdIter->first;
    return true;
    }
  if (ptIdIter == this->Internal->PointIdMap.begin())
    {
    return false;
    }

  timeStamp = (--ptIdIter)->first;
  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgTrack::GetFrameAtOrAfter(vtkVgTimeStamp& timeStamp)
{
  if (this->Internal->PointIdMap.empty())
    {
    return false;
    }

  TimeToIdMapConstIter ptIdIter;
  ptIdIter = this->Internal->PointIdMap.lower_bound(timeStamp);

  if (ptIdIter == this->Internal->PointIdMap.end())
    {
    return false;
    }

  timeStamp = ptIdIter->first;
  return true;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrack::GetNumberOfPathPoints()
{
  return static_cast<vtkIdType>(this->Internal->AllPointsIdMap.size());
}

//-----------------------------------------------------------------------------
void vtkVgTrack::InitPathTraversal()
{
  this->Internal->AllPointsIdMapIterator = this->Internal->AllPointsIdMap.begin();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgTrack::GetNextPathPt(vtkVgTimeStamp& timeStamp)
{
  if (this->Internal->AllPointsIdMapIterator != this->Internal->AllPointsIdMap.end())
    {
    timeStamp = this->Internal->AllPointsIdMapIterator->first;
    return this->Internal->AllPointsIdMapIterator++->second;
    }

  return -1;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::SetPVO(double person, double vehicle, double other)
{
  double sum = person + vehicle + other;
  if (sum != 1.0 && sum != 0.0)
    {
    person /= sum;
    vehicle /= sum;
    other /= sum;
    }
  if (this->PVO[0] == person && this->PVO[1] == vehicle && this->PVO[2] == other)
    {
    return;
    }

  this->PVO[0] = person;
  this->PVO[1] = vehicle;
  this->PVO[2] = other;

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrack::SetPVO(double pvo[3])
{
  this->SetPVO(pvo[0], pvo[1], pvo[2]);
}

//-----------------------------------------------------------------------------
int vtkVgTrack::GetBestPVOClassifier()
{
  if (this->PVO[0] == 0 && this->PVO[1] == 0 && this->PVO[2] == 0)
    {
    return vtkVgTrack::Unclassified;
    }

  if (this->PVO[0] >= this->PVO[1] && this->PVO[0] >= this->PVO[2])
    {
    return vtkVgTrack::Person;
    }
  else if (this->PVO[1] > this->PVO[0] && this->PVO[1] >= PVO[2])
    {
    return vtkVgTrack::Vehicle;
    }

  return vtkVgTrack::Other;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::SetTOC(const std::map<int, double>& toc)
{
  // Get cumulative confidence
  static const auto extractConfidence =
    [](double a, const std::pair<int, double>& p){ return a + p.second; };
  const auto sum =
    std::accumulate(toc.begin(), toc.end(), 0.0, extractConfidence);

  // If cumulative confidence is not 1.0 (within some slop), normalize the
  // input TOC and apply that instead
  if (fabs(sum - 1.0)> 1e-8)
    {
    const auto invSum = 1.0 / sum;
    std::map<int, double> normalizedToc;

    for (const auto& c : toc)
      {
      normalizedToc.emplace(c.first, c.second * invSum);
      }

    this->SetTOC(normalizedToc);
    return;
    }

  if (this->TOC == toc)
    {
    return;
    }

  this->TOC = toc;
  this->Modified();
}

//-----------------------------------------------------------------------------
std::map<int, double> vtkVgTrack::GetTOC() const
{
  return this->TOC;
}

//-----------------------------------------------------------------------------
std::pair<int, double> vtkVgTrack::GetBestTOCClassifier()
{
  auto bestClassifier = std::pair<int, double>{-1, 0.0};

  for (auto c : this->TOC)
    {
    if (c.second > bestClassifier.second)
      {
      bestClassifier = c;
      }
    }

  return bestClassifier;
}

//-----------------------------------------------------------------------------
double vtkVgTrack::GetTOCScore(int type)
{
  auto i = this->TOC.find(type);
  return (i == this->TOC.end() ? -1.0 : i->second);
}

//-----------------------------------------------------------------------------
bool vtkVgTrack::SetActiveScalars(const std::string &name)
{
  if (this->Internal->IdToScalarsMap.find(name) ==
      this->Internal->IdToScalarsMap.end())
    {
    return false;
    }

  this->Internal->ActiveScalars = this->Internal->IdToScalarsMap[name];
  this->Modified();

  return true;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgScalars> vtkVgTrack::GetActiveScalars()
{
  return this->Internal->ActiveScalars;
}

//-----------------------------------------------------------------------------
const vtkSmartPointer<vtkVgScalars> vtkVgTrack::GetActiveScalars() const
{
  return this->Internal->ActiveScalars;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::SetScalars(const std::string &name,
                            vtkVgScalars *scalars)
{
  if (!scalars || (this->Internal->IdToScalarsMap[name] == scalars))
    {
    return;
    }

  this->Internal->IdToScalarsMap[name] = scalars;
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgScalars> vtkVgTrack::GetScalars(const std::string& name)
{
  if (this->Internal->IdToScalarsMap.find(name) !=
      this->Internal->IdToScalarsMap.end())
    {
    return this->Internal->IdToScalarsMap[name];
    }
  else
    {
    return 0;
    }
}

//-----------------------------------------------------------------------------
const vtkSmartPointer<vtkVgScalars> vtkVgTrack::GetScalars(
  const std::string& name) const
{
  if (this->Internal->IdToScalarsMap.find(name) !=
      this->Internal->IdToScalarsMap.end())
    {
    return this->Internal->IdToScalarsMap[name];
    }
  else
    {
    return 0;
    }
}

//-----------------------------------------------------------------------------
std::vector<std::string> vtkVgTrack::GetScalarsName() const
{
  std::vector<std::string> names;

  std::map<std::string, Scalars>::const_iterator constItr =
    this->Internal->IdToScalarsMap.begin();
  for (; constItr != this->Internal->IdToScalarsMap.end(); ++constItr)
    {
    names.push_back(constItr->first);
    }

  return names;
}

//-----------------------------------------------------------------------------
void vtkVgTrack::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
