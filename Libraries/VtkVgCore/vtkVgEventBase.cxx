// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// VisGUI includes
#include "vtkVgEventBase.h"
#include "vtkVgTypeDefs.h"
#include "vtkVgTimeStamp.h"

// VTK includes.
#include <vtkBoundingBox.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkType.h>

#include <set>

vtkStandardNewMacro(vtkVgEventBase);
vtkCxxSetObjectMacro(vtkVgEventBase, RegionPoints, vtkPoints);

vtkImplementMetaObject(vtkVgEventTrackInfoBase, vtkVgMetaObject);

namespace // anonymous
{

//-----------------------------------------------------------------------------
vgPolygon2d makePolygon(vtkPoints* points, std::vector<vtkIdType> pointIds)
{
  vgPolygon2d polygon;

  // First point is repeated, so only consider once (ignore last point)
  for (size_t i = 0, k = pointIds.size() - 1; i < k; ++i)
    {
    double point[3];
    points->GetPoint(pointIds[i], point);
    polygon.push_back(vgPoint2d(point[0], point[1]));
    }

  return polygon;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vtkVgEventTrackInfoBase::vtkVgEventTrackInfoBase(vtkIdType trackId,
                                                 const vtkVgTimeStamp& startFrame,
                                                 const vtkVgTimeStamp& endFrame)
{
  this->TrackId = trackId;
  this->StartFrame = startFrame;
  this->EndFrame   = endFrame;
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfoBase::vtkVgEventTrackInfoBase(
  const vtkVgEventTrackInfoBase& fromTrackInfo)
  : vtkVgMetaObject(fromTrackInfo)
{
  this->TrackId = fromTrackInfo.TrackId;
  this->StartFrame = fromTrackInfo.StartFrame;
  this->EndFrame   = fromTrackInfo.EndFrame;
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfoBase* vtkVgEventTrackInfoBase::Clone() const
{
  return new vtkVgEventTrackInfoBase(*this);
}

//-----------------------------------------------------------------------------
const char* vtkVgEventTrackInfoBase::CheckValid() const
{
  if (this->TrackId < 0)
    {
    return "Only non-negative track ID's are allowed.\n";
    }

  return this->CheckBaseValid();
}

//-----------------------------------------------------------------------------
const char* vtkVgEventTrackInfoBase::CheckBaseValid() const
{
  // Check for times initialized
  if (!(this->StartFrame.IsValid() && this->EndFrame.IsValid()))
    {
    return "Start or End frame time stamp isn't valid!\n";
    }

  // Check for order
  if (this->EndFrame < this->StartFrame)
    {
    return "Start frame timestamp cannot be greater than end frame timestamp.\n";
    }

  return 0;
}

//-----------------------------------------------------------------------------
class vtkVgEventBase::vtkInternal
{
public:
  struct ClassifierInfo
    {
    double Normalcy;
    double Probability;

    ClassifierInfo()
      {
      this->Normalcy = 0.0;
      this->Probability = 0.0;
      }
    };

  std::map<int, ClassifierInfo> Classifiers;
  std::map<int, ClassifierInfo>::const_iterator ClassifierIterator;
  double ActiveClassifierScore;
  int ActiveClassifierType;

  // could use a vector instead of a map, but cleanest (At least for now)
  // to use a map (don't have to worry about missing frames, among other things)
  std::map<vtkVgTimeStamp, std::vector<vtkIdType> > RegionPtIds;
  std::map<vtkVgTimeStamp, std::vector<vtkIdType> >::iterator RegionIterator;

  std::map<vtkVgTimeStamp, vgGeocodedPoly> GeodeticRegions;

  vtkInternal()
    {
    this->ActiveClassifierScore = -1;
    this->ActiveClassifierType = -1;
    }

  double ComputeClassifierScore(const ClassifierInfo& classifierInfo) const
    {
    return (1.0 + classifierInfo.Probability) / (1.0 + classifierInfo.Normalcy);
    }

  void Copy(vtkVgEventBase::vtkInternal* source, bool copyRegionPtIds)
    {
    this->Classifiers = source->Classifiers;
    this->ClassifierIterator = this->Classifiers.end();
    this->ActiveClassifierScore = source->ActiveClassifierScore;
    this->ActiveClassifierType = source->ActiveClassifierType;

    this->GeodeticRegions = source->GeodeticRegions;

    if (copyRegionPtIds)
      {
      this->RegionPtIds = source->RegionPtIds;
      this->RegionIterator = this->RegionPtIds.end();
      }
    }

private:
  void operator=(const vtkVgEventBase::vtkInternal&);   // Not implemented.
};

//-----------------------------------------------------------------------------
vtkVgEventBase::vtkVgEventBase()
{
  this->Id = -1;
  this->Status = vgObjectStatus::None;
  this->Flags = 0;
  this->DisplayFlags = DF_TrackEvent;
  this->Name = 0;
  this->Note = 0;

  this->RegionPoints = vtkPoints::New();

  this->StartFrame.SetToMaxTime();
  this->EndFrame.SetToMinTime();

  this->TripEventPosition[0] = this->TripEventPosition[1] =
                                 this->TripEventPosition[2] = 0.0;
  this->TripEventFlag = false;

  this->Internal = new vtkInternal;
}

//-----------------------------------------------------------------------------
vtkVgEventBase::~vtkVgEventBase()
{
  delete this->Internal;
  this->SetRegionPoints(0);
  this->ClearTracks();
  this->SetNote(0);
  this->SetName(0);
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::ClearTracks()
{
  std::vector<vtkVgEventTrackInfoBase*>::const_iterator trackIter;
  for (trackIter = this->Tracks.begin(); trackIter != this->Tracks.end();
       trackIter++)
    {
    delete *trackIter;
    }
  this->Tracks.clear();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::ClearRegions()
{
  if (!this->Internal->RegionPtIds.empty())
    {
    this->Internal->RegionPtIds.clear();
    this->Internal->RegionIterator = this->Internal->RegionPtIds.end();
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::ClearGeodeticRegions()
{
  if (!this->Internal->GeodeticRegions.empty())
    {
    this->Internal->GeodeticRegions.clear();
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::AddRegion(const vtkVgTimeStamp& timeStamp,
                               vtkIdType numberOfRegionPts,
                               double* regionPts)
{
  // only add if at least 1 point AND region for this timeStamp not already set
  if (numberOfRegionPts > 0 &&
      this->Internal->RegionPtIds.find(timeStamp) ==
      this->Internal->RegionPtIds.end())
    {
    // don't process last point if same as first (we automatically duplicate it)
    if (regionPts[0] == regionPts[(numberOfRegionPts - 1) * 2] &&
        regionPts[1] == regionPts[(numberOfRegionPts - 1) * 2 + 1])
      {
      numberOfRegionPts--;
      }
    std::vector<vtkIdType> ptIds(numberOfRegionPts + 1); // + 1 to repeat 1st pt
    vtkIdType i;
    for (i = 0; i < numberOfRegionPts; i++)
      {
      ptIds.at(i) = this->RegionPoints->InsertNextPoint(
        regionPts[i * 2], regionPts[i * 2 + 1], 0.0);
      }
    ptIds.at(i) = ptIds[0];
    this->Internal->RegionPtIds[timeStamp] = ptIds;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetRegion(const vtkVgTimeStamp& timeStamp,
                               vtkIdType numberOfRegionPts,
                               double* regionPts)
{
  // only add if at least 1 point (this fn can NOT be used to remove a region)
  if (numberOfRegionPts <= 0)
    {
    return;
    }

  std::map<vtkVgTimeStamp, std::vector<vtkIdType> >::iterator region =
    this->Internal->RegionPtIds.find(timeStamp);
  if (region == this->Internal->RegionPtIds.end())
    {
    // if region not already set, use AddRegion
    this->AddRegion(timeStamp, numberOfRegionPts, regionPts);
    return;
    }

  // don't process last point if same as first (we automatically duplicate it)
  if (regionPts[0] == regionPts[(numberOfRegionPts - 1) * 2] &&
      regionPts[1] == regionPts[(numberOfRegionPts - 1) * 2 + 1])
    {
    numberOfRegionPts--;
    }

  // replace existing region with the update, IF there is a change
  bool regionChanged = false;
  size_t numberOfExistingRegionPts = region->second.size();
  // "- 1" because we duplicate the 1st point at the end (and incoming points
  // have already been decremented accordingly)
  if (numberOfExistingRegionPts - 1 == numberOfRegionPts)
    {
    for (vtkIdType i = 0; i < numberOfRegionPts; ++i)
      {
      double* point = this->RegionPoints->GetPoint(region->second[i]);
      if (point[0] != regionPts[i * 2] || point[1] != regionPts[i * 2 + 1])
        {
        regionChanged = true;
        break;
        }
      }
    }
  else
    {
    regionChanged = true;
    }

  if (!regionChanged)  // no change
    {
    return;
    }

  if (numberOfRegionPts + 1 != numberOfExistingRegionPts)
    {
    // if new size of region is smaller than original, the difference is wasted
    region->second.resize(numberOfRegionPts + 1);
    region->second.at(numberOfRegionPts) = region->second[0];
    }
  for (vtkIdType i = 0; i < numberOfRegionPts; ++i)
    {
    if (i < numberOfExistingRegionPts)
      {
      this->RegionPoints->SetPoint(region->second[i], regionPts[i * 2],
                                                      regionPts[i * 2 + 1],
                                                      0.0);
      }
    else
      {
      region->second.at(i) =
        this->RegionPoints->InsertNextPoint(regionPts[i * 2],
                                            regionPts[i * 2 + 1], 0.0);
      }
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::GetRegion(const vtkVgTimeStamp& timeStamp,
                               vtkIdType& npts, vtkIdType*& pts)
{
  std::map<vtkVgTimeStamp, std::vector<vtkIdType> >::iterator region =
    this->Internal->RegionPtIds.find(timeStamp);
  if (region == this->Internal->RegionPtIds.end())
    {
    npts = 0;
    }
  else
    {
    npts = static_cast<vtkIdType>(region->second.size());
    pts = &region->second[0];
    }
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::GetRegionAtOrAfter(vtkVgTimeStamp& timeStamp,
                                        vtkIdType& npts, vtkIdType*& pts)
{
  std::map<vtkVgTimeStamp, std::vector<vtkIdType> >::iterator region
    = this->Internal->RegionPtIds.lower_bound(timeStamp);
  if (region == this->Internal->RegionPtIds.end())
    {
    npts = 0;
    return false;
    }
  else
    {
    npts = static_cast<vtkIdType>(region->second.size());
    pts = &region->second[0];
    if (!(region->first == timeStamp))
      {
      timeStamp = region->first;
      return false;
      }
    return true;
    }
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::GetClosestDisplayRegion(vtkVgTimeStamp& timeStamp,
                                             vtkIdType& npts, vtkIdType*& pts)
{
  if (this->Internal->RegionPtIds.empty())
    {
    // No regions in the map
    npts = 0;
    return false;
    }

  std::map<vtkVgTimeStamp, std::vector<vtkIdType> >::iterator region
    = this->Internal->RegionPtIds.upper_bound(timeStamp);
  if (region != this->Internal->RegionPtIds.begin())
    {
    --region;
    }

  npts = static_cast<vtkIdType>(region->second.size());
  pts = &region->second[0];
  if (!(region->first == timeStamp))
    {
    timeStamp = region->first;
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::GetRegionCenter(const vtkVgTimeStamp& timeStamp,
                                     double* center,
                                     bool interpolated)
{
  if (this->Internal->RegionPtIds.empty())
    {
    return false;
    }

  interpolated = false;  // not intperolated until we actual do (interpolate)

  // get reg
  std::map<vtkVgTimeStamp, std::vector<vtkIdType> >::iterator region;
  region = this->Internal->RegionPtIds.lower_bound(timeStamp);
  if (region == this->Internal->RegionPtIds.end())
    {
    interpolated = true;
    region--;
    }
  else if (timeStamp < this->Internal->RegionPtIds.begin()->first)
    {
    interpolated = true;
    }

  vtkBoundingBox bbox;
  // 1st pt is repeated, so only consider once (ignore last point)
  for (size_t i = 0; i < region->second.size() - 1; i++)
    {
    bbox.AddPoint(this->RegionPoints->GetPoint(region->second[i]));
    }
  double center3D[3];
  bbox.GetCenter(center3D);

  // if interpolated == true (already), means either before 1st region, or
  // after last, in which case we use the exact pt
  if (region->first == timeStamp || interpolated)
    {
    center[0] = center3D[0];
    center[1] = center3D[1];
    }
  else // need to use preceding region and interpolate
    {
    vtkVgTimeStamp afterTimeStamp = timeStamp;

    region--;
    bbox.Reset();
    // 1st pt is repeated, so only consider once (ignore last point)
    for (size_t i = 0; i < region->second.size() - 1; i++)
      {
      bbox.AddPoint(this->RegionPoints->GetPoint(region->second[i]));
      }
    double centerBefore[3];
    bbox.GetCenter(centerBefore);

    double t;
    if (timeStamp.HasTime())
      {
      t = (timeStamp.GetTime() - region->first.GetTime()) /
          (afterTimeStamp.GetTime() - region->first.GetTime());
      }
    else
      {
      t = (timeStamp.GetFrameNumber() - region->first.GetFrameNumber()) /
          (double)(afterTimeStamp.GetFrameNumber() - region->first.GetFrameNumber());
      }

    center[0] = centerBefore[0] + t * (center3D[0] - centerBefore[0]);
    center[1] = centerBefore[1] + t * (center3D[1] - centerBefore[1]);
    }

  return true;
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::AddRegion(const vtkVgTimeStamp& timeStamp,
                               const vgPolygon2d& region)
{
  size_t numberOfRegionPts = region.size();

  // only add if at least 1 point AND region for this timeStamp not already set
  if (numberOfRegionPts > 0 &&
      this->Internal->RegionPtIds.find(timeStamp) ==
      this->Internal->RegionPtIds.end())
    {
    // don't process last point if same as first (we automatically duplicate it)
    if (region[0] == region[numberOfRegionPts - 1])
      {
      numberOfRegionPts--;
      }
    std::vector<vtkIdType> ptIds(numberOfRegionPts + 1); // + 1 to repeat first point
    size_t i;
    for (i = 0; i < numberOfRegionPts; ++i)
      {
      ptIds[i] = this->RegionPoints->InsertNextPoint(
        region[i].X, region[i].Y, 0.0);
      }
    ptIds[i] = ptIds[0];
    this->Internal->RegionPtIds[timeStamp] = ptIds;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vgPolygon2d vtkVgEventBase::GetRegion(const vtkVgTimeStamp& timeStamp) const
{
  typedef std::vector<vtkIdType> PtIdList;
  typedef std::map<vtkVgTimeStamp, PtIdList> PtMap;
  typedef PtMap::const_iterator Iterator;

  Iterator region = this->Internal->RegionPtIds.find(timeStamp);
  if (region != this->Internal->RegionPtIds.end())
    {
    return makePolygon(this->RegionPoints, region->second);
    }

  return vgPolygon2d();
}

//-----------------------------------------------------------------------------
vgPolygon2d vtkVgEventBase::GetRegionAtOrAfter(vtkVgTimeStamp& timeStamp) const
{
  typedef std::vector<vtkIdType> PtIdList;
  typedef std::map<vtkVgTimeStamp, PtIdList> PtMap;
  typedef PtMap::const_iterator Iterator;

  vgPolygon2d result;

  Iterator region = this->Internal->RegionPtIds.lower_bound(timeStamp);
  if (region != this->Internal->RegionPtIds.end())
    {
    timeStamp = region->first;
    return makePolygon(this->RegionPoints, region->second);
    }

  return vgPolygon2d();
}

//-----------------------------------------------------------------------------
std::map<vtkVgTimeStamp, vgPolygon2d> vtkVgEventBase::GetRegions() const
{
  typedef std::vector<vtkIdType> PtIdList;
  typedef std::map<vtkVgTimeStamp, PtIdList> PtMap;
  typedef PtMap::const_iterator Iterator;

  Iterator iter = this->Internal->RegionPtIds.begin();
  const Iterator end = this->Internal->RegionPtIds.end();

  std::map<vtkVgTimeStamp, vgPolygon2d> result;

  while (iter != end)
    {
    const vgPolygon2d& region = makePolygon(this->RegionPoints, iter->second);
    result.insert(std::make_pair(iter->first, region));
    ++iter;
    }

  return result;
}

//-----------------------------------------------------------------------------
std::size_t vtkVgEventBase::GetNumberOfRegions() const
{
  return this->Internal->RegionPtIds.size();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::InitRegionTraversal()
{
  this->Internal->RegionIterator = this->Internal->RegionPtIds.begin();
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::GetNextRegion(vtkVgTimeStamp& timeStamp, vtkIdType& npts,
                                   vtkIdType*& pts)
{
  if (this->Internal->RegionIterator == this->Internal->RegionPtIds.end())
    {
    return false;
    }

  timeStamp = this->Internal->RegionIterator->first;
  npts = static_cast<vtkIdType>(this->Internal->RegionIterator->second.size());
  pts = &this->Internal->RegionIterator->second[0];

  ++this->Internal->RegionIterator;
  return true;
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::AddGeodeticRegion(const vtkVgTimeStamp& timeStamp,
                                       const vgGeocodedPoly& region)
{
  if (timeStamp.IsValid() && region.GCS != -1 && !region.Coordinate.empty())
    {
    this->Internal->GeodeticRegions.insert(std::make_pair(timeStamp, region));
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
std::map<vtkVgTimeStamp, vgGeocodedPoly>
vtkVgEventBase::GetGeodeticRegions() const
{
  return this->Internal->GeodeticRegions;
}

//-----------------------------------------------------------------------------
std::size_t vtkVgEventBase::GetNumberOfGeodeticRegions() const
{
  return this->Internal->GeodeticRegions.size();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::AddClassifier(int type,
                                   double probability/*=0*/,
                                   double normalcy/*=0*/)
{
  vtkInternal::ClassifierInfo classifierInfo;

  if (normalcy < 0.0 || normalcy > 1.0)
    {
    vtkWarningMacro("Event normalcy out of range: " << this->GetId());
    normalcy = normalcy < 0.0 ? 0.0 : (normalcy > 1.0 ? 1.0 : normalcy);
    }

  classifierInfo.Normalcy = normalcy;
  classifierInfo.Probability = probability;

  this->Internal->Classifiers[type] = classifierInfo;
  this->Modified();

  double score = this->Internal->ComputeClassifierScore(classifierInfo);
  if (score > this->Internal->ActiveClassifierScore)
    {
    this->Internal->ActiveClassifierScore = score;
    this->Internal->ActiveClassifierType = type;
    }
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::RemoveClassifier(int type)
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifier =
    this->Internal->Classifiers.find(type);
  if (classifier != this->Internal->Classifiers.end())
    {
    this->Internal->Classifiers.erase(classifier);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::ResetClassifiers()
{
  this->Internal->Classifiers.clear();
  this->ResetActiveClassifier();
  this->Modified();
}

//-----------------------------------------------------------------------------
unsigned int vtkVgEventBase::GetNumberOfClassifiers()
{
  return static_cast<unsigned int>(this->Internal->Classifiers.size());
}

//-----------------------------------------------------------------------------
std::vector<int> vtkVgEventBase::GetClassifierTypes()
{
  std::vector<int> types;
  types.reserve(this->Internal->Classifiers.size());

  std::map<int, vtkInternal::ClassifierInfo>::iterator iter, end;
  for (iter = this->Internal->Classifiers.begin(),
       end  = this->Internal->Classifiers.end();
       iter != end; ++iter)
    {
    types.push_back(iter->first);
    }

  return types;
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::InitClassifierTraversal()
{
  this->Internal->ClassifierIterator = this->Internal->Classifiers.begin();
  return this->Internal->ClassifierIterator != this->Internal->Classifiers.end();
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::NextClassifier()
{
  if (this->Internal->ClassifierIterator == this->Internal->Classifiers.end())
    {
    return false;
    }
  this->Internal->ClassifierIterator++;

  return this->Internal->ClassifierIterator != this->Internal->Classifiers.end();
}

//-----------------------------------------------------------------------------
int vtkVgEventBase::GetClassifierType()
{
  if (this->Internal->ClassifierIterator == this->Internal->Classifiers.end())
    {
    vtkErrorMacro("No more classifiers!");
    return -1;
    }

  return this->Internal->ClassifierIterator->first;
}

//-----------------------------------------------------------------------------
double vtkVgEventBase::GetClassifierProbability()
{
  if (this->Internal->ClassifierIterator == this->Internal->Classifiers.end())
    {
    vtkErrorMacro("No more classifiers!");
    return -1;
    }

  return this->Internal->ClassifierIterator->second.Probability;
}

//-----------------------------------------------------------------------------
double vtkVgEventBase::GetClassifierNormalcy()
{
  if (this->Internal->ClassifierIterator == this->Internal->Classifiers.end())
    {
    vtkErrorMacro("No more classifiers!");
    return -1;
    }

  return this->Internal->ClassifierIterator->second.Normalcy;
}

//-----------------------------------------------------------------------------
double vtkVgEventBase::GetClassifierScore()
{
  if (this->Internal->ClassifierIterator == this->Internal->Classifiers.end())
    {
    vtkErrorMacro("No more classifiers!");
    return -1;
    }

  return this->Internal->ComputeClassifierScore(
           this->Internal->ClassifierIterator->second);
}
//-----------------------------------------------------------------------------
bool vtkVgEventBase::HasClassifier(int classifierType)
{
  return this->Internal->Classifiers.find(classifierType) !=
         this->Internal->Classifiers.end();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetProbability(int classifierType, double probability)
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifier =
    this->Internal->Classifiers.find(classifierType);
  if (classifier == this->Internal->Classifiers.end() ||
      classifier->second.Probability == probability)
    {
    return;
    }

  classifier->second.Probability = probability;
  double score = this->Internal->ComputeClassifierScore(classifier->second);
  if (score > this->Internal->ActiveClassifierScore)
    {
    this->Internal->ActiveClassifierScore = score;
    this->Internal->ActiveClassifierType = classifierType;
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
double vtkVgEventBase::GetProbability(int classifierType)
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifier =
    this->Internal->Classifiers.find(classifierType);
  if (classifier == this->Internal->Classifiers.end())
    {
    return -1;
    }
  return classifier->second.Probability;
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetNormalcy(int classifierType, double normalcy)
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifier =
    this->Internal->Classifiers.find(classifierType);
  if (classifier == this->Internal->Classifiers.end() ||
      classifier->second.Normalcy == normalcy)
    {
    return;
    }

  classifier->second.Normalcy = normalcy;
  double score = this->Internal->ComputeClassifierScore(classifier->second);
  if (score > this->Internal->ActiveClassifierScore)
    {
    this->Internal->ActiveClassifierScore = score;
    this->Internal->ActiveClassifierType = classifierType;
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
double vtkVgEventBase::GetNormalcy(int classifierType)
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifier =
    this->Internal->Classifiers.find(classifierType);
  if (classifier == this->Internal->Classifiers.end())
    {
    return -1;
    }
  return classifier->second.Normalcy;
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetActiveClassifier(int classifierType)
{
  if (this->Internal->ActiveClassifierType == classifierType)
    {
    return;
    }

  if (this->Internal->Classifiers.find(classifierType) ==
      this->Internal->Classifiers.end())
    {
    vtkErrorMacro("Classifier of type " << classifierType <<
                  " is not present in this event");
    }

  this->Internal->ActiveClassifierType = classifierType;
  this->Internal->ActiveClassifierScore = VTK_FLOAT_MAX;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::ResetActiveClassifier()
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifierIter =
    this->Internal->Classifiers.begin();
  double bestScore = -1;
  int bestClassifier = -1;
  for (; classifierIter != this->Internal->Classifiers.end(); classifierIter++)
    {
    double score = this->Internal->ComputeClassifierScore(classifierIter->second);
    if (score > bestScore)
      {
      bestScore = score;
      bestClassifier = classifierIter->first;
      }
    }
  this->Internal->ActiveClassifierType = bestClassifier;
  this->Internal->ActiveClassifierScore = bestScore;
}

//-----------------------------------------------------------------------------
int vtkVgEventBase::GetActiveClassifierType()
{
  return this->Internal->ActiveClassifierType;
}

//-----------------------------------------------------------------------------
double vtkVgEventBase::GetActiveClassifierNormalcy()
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifier =
    this->Internal->Classifiers.find(this->Internal->ActiveClassifierType);
  if (classifier == this->Internal->Classifiers.end())
    {
    return -1;
    }
  return classifier->second.Normalcy;
}

//-----------------------------------------------------------------------------
double vtkVgEventBase::GetActiveClassifierProbability()
{
  std::map<int, vtkInternal::ClassifierInfo>::iterator classifier =
    this->Internal->Classifiers.find(this->Internal->ActiveClassifierType);
  if (classifier == this->Internal->Classifiers.end())
    {
    return -1;
    }
  return classifier->second.Probability;
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::AddTrack(vtkIdType trackId, vtkVgTimeStamp& startFrame,
                              vtkVgTimeStamp& endFrame)
{
  vtkVgEventTrackInfoBase* trackInfo =
    new vtkVgEventTrackInfoBase(trackId, startFrame, endFrame);

  this->AddTrack(trackInfo);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::AddTrack(vtkVgEventTrackInfoBase* trackInfo)
{
  // check for valid info
  const char* error = trackInfo->CheckValid();
  if (error)
    {
    vtkErrorMacro(<< error);
    return;
    }

  // update event start/end given this track
  if (trackInfo->StartFrame < this->StartFrame)
    {
    this->StartFrame = trackInfo->StartFrame;
    }
  if (this->EndFrame < trackInfo->EndFrame)
    {
    this->EndFrame = trackInfo->EndFrame;
    }

  this->Tracks.push_back(trackInfo);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::RemoveTrack(unsigned int index)
{
  delete this->Tracks[index];
  this->Tracks.erase(this->Tracks.begin() + index);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SwapTracks(unsigned int index1, unsigned int index2)
{
  std::swap(this->Tracks[index1], this->Tracks[index2]);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::ReplaceTrack(unsigned int index, vtkIdType trackId)
{
  this->Tracks[index]->TrackId = trackId;
  this->Modified();
}

//-----------------------------------------------------------------------------
unsigned int vtkVgEventBase::GetNumberOfTracks() const
{
  return static_cast<unsigned int>(this->Tracks.size());
}

//-----------------------------------------------------------------------------
unsigned int vtkVgEventBase::GetNumberOfTrackGroups() const
{
  // initial implementation: all tracks of same id are grouped together
  std::vector<vtkVgEventTrackInfoBase*>::const_iterator trackIter;
  std::set<vtkIdType> uniqueTracks;
  for (trackIter = this->Tracks.begin(); trackIter != this->Tracks.end();
       trackIter++)
    {
    uniqueTracks.insert((*trackIter)->TrackId);
    }
  return static_cast<unsigned int>(uniqueTracks.size());
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::GetTrack(unsigned int index, vtkIdType& trackId,
                              vtkVgTimeStamp& startFrame,
                              vtkVgTimeStamp& endFrame)
{
  if (index >= this->Tracks.size())
    {
    vtkErrorMacro("Index out of range.\n");
    return;
    }

  vtkVgEventTrackInfoBase* trackInfo = this->Tracks[index];
  trackId         = trackInfo->TrackId;
  startFrame      = trackInfo->StartFrame;
  endFrame        = trackInfo->EndFrame;
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgEventBase::GetTrackId(unsigned int index) const
{
  if (index >= this->Tracks.size())
    {
    return -1;
    }

  return this->Tracks[index]->TrackId;
}

//-----------------------------------------------------------------------------
vtkVgEventTrackInfoBase* vtkVgEventBase::GetTrackInfo(unsigned int index) const
{
  if (index >= this->Tracks.size())
    {
    return 0;
    }

  return this->Tracks[index];
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::DeepCopy(vtkVgEventBase* src, bool appendRegionPoints)
{
  if (src)
    {
    this->CopyEventInfo(src);

    // event Id
    this->Id = src->Id;

    if (appendRegionPoints)
      {
      std::map<vtkVgTimeStamp, std::vector<vtkIdType> >::iterator regionIter;
      for (regionIter = this->Internal->RegionPtIds.begin();
           regionIter != this->Internal->RegionPtIds.end(); regionIter++)
        {
        std::vector<vtkIdType>::iterator ptIdIter;
        for (ptIdIter = regionIter->second.begin();
             ptIdIter != regionIter->second.end() - 1; ptIdIter++)
          {
          *ptIdIter = this->RegionPoints->InsertNextPoint(
                        src->RegionPoints->GetPoint(*ptIdIter));
          }
        *ptIdIter = *regionIter->second.begin();
        }
      }
    else
      {
      this->RegionPoints->DeepCopy(src->RegionPoints);
      }

    this->Modified();
    }
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::Merge(vtkVgEventBase* other)
{
  if (this->GetNumberOfRegions() != 0 || other->GetNumberOfRegions() != 0)
    {
    vtkErrorMacro("Merging region-based events is not yet supported.");
    return false;
    }

  if (this->GetNumberOfGeodeticRegions() != 0 ||
      other->GetNumberOfGeodeticRegions() != 0)
    {
    vtkErrorMacro("Merging region-based events is not yet supported.");
    return false;
    }

  if (this->GetNumberOfTracks() != 1 || other->GetNumberOfTracks() != 1)
    {
    vtkErrorMacro("Cannot merge multi-track events");
    return false;
    }

  if (this->Internal->Classifiers.size() != other->Internal->Classifiers.size())
    {
    vtkErrorMacro("Cannot merge events with a differing number of classifiers.");
    return false;
    }

  if (this->Internal->Classifiers.size() > 1 ||
      other->Internal->Classifiers.size() > 1)
    {
    vtkErrorMacro("Cannot merge events with multiple classifiers.");
    return false;
    }

  if (!this->Internal->Classifiers.empty() &&
      this->Internal->Classifiers.begin()->first !=
      other->Internal->Classifiers.begin()->first)
    {
    vtkErrorMacro("Cannot merge events with different classifier types.");
    return false;
    }

  // Lengthen the track to cover the extents in both events.
  // TODO: Do something with classifier score?
  if (other->Tracks[0]->StartFrame < this->Tracks[0]->StartFrame)
    {
    this->SetTrackStartFrame(0, other->Tracks[0]->StartFrame);
    }
  if (other->Tracks[0]->EndFrame > this->Tracks[0]->EndFrame)
    {
    this->SetTrackEndFrame(0, other->Tracks[0]->EndFrame);
    }

  return true;
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::CopyTracks(std::vector<vtkVgEventTrackInfoBase*>& tracks)
{
  std::vector<vtkVgEventTrackInfoBase*>::const_iterator trackIter;
  for (trackIter = tracks.begin();
       trackIter != tracks.end(); trackIter++)
    {
    vtkVgEventTrackInfoBase* trackInfo = (*trackIter)->Clone();
    this->Tracks.push_back(trackInfo);
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetTrackStartFrame(unsigned int trackIndex,
                                        const vtkVgTimeStamp& startFrame)
{
  if (trackIndex >= this->GetNumberOfTracks())
    {
    vtkErrorMacro("Index out of range.\n");
    return;
    }

  if (this->Tracks[trackIndex]->StartFrame == startFrame)
    {
    return;
    }

  this->Tracks[trackIndex]->StartFrame = startFrame;

  this->UpdateEventFrameBounds();
  this->Flags |= EF_Dirty;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetTrackEndFrame(unsigned int trackIndex,
                                      const vtkVgTimeStamp& endFrame)
{
  if (trackIndex >= this->GetNumberOfTracks())
    {
    vtkErrorMacro("Index out of range.\n");
    return;
    }

  if (this->Tracks[trackIndex]->EndFrame == endFrame)
    {
    return;
    }

  this->Tracks[trackIndex]->EndFrame = endFrame;

  this->UpdateEventFrameBounds();
  this->Flags |= EF_Dirty;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::UpdateEventFrameBounds()
{
  if (this->GetNumberOfTracks() == 0)
    {
    return;
    }

  const vtkVgEventTrackInfoBase* track0 = this->Tracks[0];

  vtkVgTimeStamp minStartFrame = track0->StartFrame;
  vtkVgTimeStamp maxEndFrame = track0->EndFrame;

  for (unsigned int i = 1; i < this->GetNumberOfTracks(); ++i)
    {
    const vtkVgEventTrackInfoBase* track = this->Tracks[i];
    if (track->StartFrame < minStartFrame)
      {
      minStartFrame = track->StartFrame;
      }
    if (maxEndFrame < track->EndFrame)
      {
      maxEndFrame = track->EndFrame;
      }
    }

  // if values are unchanged, then "Set" won't do anything
  this->SetStartFrame(minStartFrame);
  this->SetEndFrame(maxEndFrame);
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetStartFrame(const vtkVgTimeStamp& timeStamp)
{
  if (this->StartFrame == timeStamp)
    {
    return;
    }

  this->StartFrame = timeStamp;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetEndFrame(const vtkVgTimeStamp& timeStamp)
{
  if (this->EndFrame == timeStamp)
    {
    return;
    }

  this->EndFrame = timeStamp;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::ClearTripEvent()
{
  if (this->TripEventFlag)
    {
    this->TripEventFlag = false;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::SetTripEventInfo(double tripLocation[3],
                                      const vtkVgTimeStamp& tripTime)
{
  this->TripEventFlag = true;
  this->SetTripEventPosition(tripLocation);
  this->TripEventTime = tripTime;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::UpdateEvent(vtkVgEventBase* fromEvent)
{
  // the one check we make... is that the Id matches the previous Id
  if (fromEvent->GetId() != this->Id)
    {
    vtkErrorMacro("Can only update event if source Id matches original Id");
    return;
    }

  this->CopyEventInfo(fromEvent, false);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::CloneEvent(vtkVgEventBase* fromEvent)
{
  this->CopyEventInfo(fromEvent);
  this->SetRegionPoints(fromEvent->GetRegionPoints());
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgEventBase::CopyEventInfo(vtkVgEventBase* fromEvent,
                                   bool copyRegionPtIds)
{
  this->ClearTracks();
  this->CopyTracks(fromEvent->Tracks);

  // event start and end info
  this->StartFrame = fromEvent->StartFrame;
  this->EndFrame = fromEvent->EndFrame;

  this->Status = fromEvent->Status;
  this->Flags = fromEvent->Flags;
  this->SetName(fromEvent->GetName());
  this->SetNote(fromEvent->GetNote());

  // Trip event info
  this->TripEventFlag = fromEvent->TripEventFlag;
  this->SetTripEventPosition(fromEvent->TripEventPosition);
  this->TripEventTime = fromEvent->TripEventTime;

  // and the internals (classifiers and regions)
  this->Internal->Copy(fromEvent->Internal, copyRegionPtIds);

  // calling function left to handle the Modified state
}

//-----------------------------------------------------------------------------
bool vtkVgEventBase::IsTimeWithinExtents(const vtkVgTimeStamp& timeStamp)
{
  // TODO: This seems either unnecessary or wrong. Investigate why the ctor
  //       defaults the start/end to min_time/max_time.
  bool hasStart = this->StartFrame.IsValid() && !this->StartFrame.IsMaxTime();
  bool hasEnd = this->EndFrame.IsValid() && !this->EndFrame.IsMinTime();

  if ((!hasStart || timeStamp >= this->StartFrame) &&
      (!hasEnd || timeStamp <= this->EndFrame))
    {
    return true;
    }
  return false;
}
