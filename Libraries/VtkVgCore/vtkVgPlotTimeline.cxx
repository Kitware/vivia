/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgPlotTimeline.h"

#include "vtkIdTypeArray.h"
#include "vtkContext2D.h"
#include "vtkContextDevice2D.h"
#include "vtkObjectFactory.h"
#include "vtkPen.h"
#include "vtkPoints2D.h"
#include "vtkTransform2D.h"
#include "vtkUnsignedCharArray.h"

#include <algorithm>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVgPlotTimeline);

//-----------------------------------------------------------------------------
vtkVgPlotTimeline::vtkVgPlotTimeline()
{
  this->MarkerStyle = vtkPlotPoints::CIRCLE;
  this->SortedIds = 0;
  this->AreaPointIds = 0;
  this->IsIntervalPlot = false;
}

//-----------------------------------------------------------------------------
vtkVgPlotTimeline::~vtkVgPlotTimeline()
{
  delete [] this->SortedIds;

  if (this->AreaPointIds)
    {
    this->AreaPointIds->Delete();
    }
}

//-----------------------------------------------------------------------------
void vtkVgPlotTimeline::SetSelection(vtkIdTypeArray* id)
{
  if (!this->Selection)
    {
    this->Selection = vtkIdTypeArray::New();
    }
  if (!id)
    {
    this->Selection->Reset();
    }
  else
    {
    this->Selection->DeepCopy(id);
    }
}

//-----------------------------------------------------------------------------
bool vtkVgPlotTimeline::Paint(vtkContext2D* painter)
{
  if (!this->Visible || !this->Points)
    {
    return false;
    }

  // get scaling factor to convert input item dimensions to screen space
  float widthScale = painter->GetTransform()->GetMatrix()->GetData()[0];
  float minScreenWidth = 5.0f;

  // don't try to draw a line with <2 points
  vtkIdType numPoints = this->Points->GetNumberOfPoints();
  if (numPoints > 1)
    {
    // interval plot requires a special paint sequence
    if (this->IsIntervalPlot)
      {
      painter->ApplyPen(this->Pen);

      vtkVector2f* data =
        static_cast<vtkVector2f*>(this->Points->GetVoidPointer(0));

      // coloring with scalars?
      if (this->ScalarVisibility && this->Colors)
        {
        int nc = static_cast<int>(this->Colors->GetNumberOfTuples());
        if (numPoints != nc)
          {
          vtkErrorMacro("Attempted to color points with array of wrong length");
          return false;
          }

        int nc_comps = static_cast<int>(this->Colors->GetNumberOfComponents());
        unsigned char* c = this->Colors->GetPointer(0);

        painter->GetPen()->SetWidth(5);

        // draw intervals
        for (vtkIdType i = 0; i < numPoints - 1; i += 2)
          {
          float points[4] =
            {
            data[i].GetX(),
            data[i].GetY(),
            data[i + 1].GetX(),
            data[i + 1].GetY()
            };

          // Enforce a minimum screen width to ensure the item remains visible.
          if (widthScale > 0.0f)
            {
            float screenWidth = (points[2] - points[0]) * widthScale;
            if (screenWidth < minScreenWidth)
              {
              float halfWidth = 0.5f * minScreenWidth / widthScale;
              float mid = 0.5f * (points[0] + points[2]);
              points[0] = mid - halfWidth;
              points[2] = mid + halfWidth;
              }
            }

          painter->DrawPoly(points, 2, c, nc_comps);
          c += nc_comps * 2;
          }
        }
      else // not coloring with scalars
        {
        for (vtkIdType i = 0; i < numPoints - 1; i += 2)
          {
          painter->DrawLine(data[i].GetX(),
                            data[i].GetY(),
                            data[i + 1].GetX(),
                            data[i + 1].GetY());
          }
        }
      }
    else
      {
      return this->vtkPlotLine::Paint(painter);
      }
    }

  if (!this->IsIntervalPlot)
    {
    return this->vtkPlotPoints::Paint(painter);
    }

  // draw interval selection highlight
  if (this->Selection)
    {
    vtkDebugMacro(<< "Selection set " << this->Selection->GetNumberOfTuples());

    painter->GetPen()->SetColor(0, 0, 255, 80);
    painter->GetPen()->SetWidth(13);

    for (int i = 0; i < this->Selection->GetNumberOfTuples(); ++i)
      {
      vtkIdType id = 0;
      this->Selection->GetTupleValue(i, &id);
      if (id % 2 == 0 && id < this->Points->GetNumberOfPoints() - 1)
        {
        double p[4];
        this->Points->GetPoint(id, p);
        this->Points->GetPoint(id + 1, p + 2);

        // enforce min screen width
        if (widthScale > 0.0f)
          {
          float screenWidth = (p[2] - p[0]) * widthScale;
          if (screenWidth < minScreenWidth)
            {
            float halfWidth = 0.5f * minScreenWidth / widthScale;
            float mid = 0.5f * (p[0] + p[2]);
            p[0] = mid - halfWidth;
            p[2] = mid + halfWidth;
            }
          }

        painter->DrawLine(p[0], p[1], p[2], p[3]);
        }
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
namespace
{

// compare the x component of the two points corresponding to two ids
struct CompareIds
{
  vtkVector2f* Points;

  CompareIds(vtkVector2f* p) : Points(p) { }

  bool operator()(vtkIdType a, vtkIdType b)
    {
    return this->Points[a].GetX() < this->Points[b].GetX();
    }
};

// define a minimal iterator to allow binary search
// of point ids by their x value
struct IdToPointXIter
    : public std::iterator<std::forward_iterator_tag, double>
{
  vtkVector2f* Points;
  vtkIdType*   Pointer;

  IdToPointXIter(vtkVector2f* pts = 0, vtkIdType* start = 0)
    : Points(pts), Pointer(start)
    {}

  double operator*()
    { return this->Points[*Pointer].GetX(); }

  IdToPointXIter& operator++()
    { ++this->Pointer; return *this; }

  bool operator==(const IdToPointXIter& other)
    { return this->Pointer == other.Pointer; }

  bool operator!=(const IdToPointXIter& other)
    { return !(*this == other); }
};

// fills a sequence of length n with values [0, 1, 2, ..., n-1]
struct GenSeries
{
  vtkIdType Current;
  GenSeries() : Current(0)  { }
  vtkIdType operator()() { return this->Current++; }
};

} // end anonymous namespace

//-----------------------------------------------------------------------------
bool vtkVgPlotTimeline::SelectIntervals(const vtkVector2f& point,
                                        const vtkVector2f& tol)
{
  if (!this->Points)
    {
    return false;
    }

  if (!this->Selection)
    {
    this->Selection = vtkIdTypeArray::New();
    }
  this->Selection->SetNumberOfTuples(0);

  // Iterate through all points and check whether any are in range
  vtkVector2f* data = static_cast<vtkVector2f*>(
                        this->Points->GetVoidPointer(0));
  vtkIdType n = this->Points->GetNumberOfPoints();

  for (vtkIdType i = 0; i < n - 1; i += 2)
    {
    if (this->InRange(point, tol, data[i], data[i + 1]))
      {
      // only add the 'head' id to the selection
      this->Selection->InsertNextValue(i);
      }
    }
  return this->Selection->GetNumberOfTuples() > 0;
}

//-----------------------------------------------------------------------------
bool vtkVgPlotTimeline::SelectPoints(const vtkVector2f& min,
                                     const vtkVector2f& max)
{
  if (!this->IsIntervalPlot)
    {
    return this->Superclass::SelectPoints(min, max);
    }

  vtkVector2f pt;
  vtkVector2f tol;
  pt[0] = 0.5 * (min.GetX() + max.GetX());
  pt[1] = 0.5 * (min.GetY() + max.GetY());
  tol[0] = max.GetX() - pt[0];
  tol[1] = max.GetY() - pt[1];
  return this->SelectIntervals(pt, tol);
}

//-----------------------------------------------------------------------------
vtkIdTypeArray* vtkVgPlotTimeline::GetIntervalIdsInArea(
  const vtkVector2f& point, const vtkVector2f& tol)
{
  if (!this->Points)
    {
    return 0;
    }

  vtkIdType n = this->Points->GetNumberOfPoints();
  if (n == 0)
    {
    return 0;
    }

  vtkVector2f* points = static_cast<vtkVector2f*>(
                          this->Points->GetVoidPointer(0));

  // sort ids according to the x values of the corresponding points
  if (!this->SortedIds || this->SortTime < this->BuildTime)
    {
    this->SortTime.Modified();
    delete [] this->SortedIds;
    this->SortedIds = new vtkIdType[n];
    std::generate(this->SortedIds, this->SortedIds + n, GenSeries());
    std::sort(this->SortedIds, this->SortedIds + n, CompareIds(points));
    }

  // get the lowest point we might hit within the supplied tolerance
  double lowX = point.GetX() - tol.GetX();

  IdToPointXIter begin(points, this->SortedIds);
  IdToPointXIter end(points, this->SortedIds + n);
  IdToPointXIter low = std::lower_bound(begin, end, lowX);

  // set up the array
  if (!this->AreaPointIds)
    {
    this->AreaPointIds = vtkIdTypeArray::New();
    }
  this->AreaPointIds->SetNumberOfTuples(0);

  // now consider the y axis
  for (; low != end; ++low)
    {
    // only consider interval end points
    vtkIdType currId = *(low.Pointer);
    if (currId % 2 == 1)
      {
      if (this->InRange(point, tol, points[currId - 1], points[currId]))
        {
        this->AreaPointIds->InsertNextValue(currId);
        };
      }
    }

  return this->AreaPointIds;
}

//-----------------------------------------------------------------------------
vtkIdTypeArray* vtkVgPlotTimeline::GetPointIdsInArea(const vtkVector2f& point,
    const vtkVector2f& tol)
{
  if (!this->Points)
    {
    return 0;
    }

  vtkIdType n = this->Points->GetNumberOfPoints();
  if (n == 0)
    {
    return 0;
    }

  vtkVector2f* points = static_cast<vtkVector2f*>(
                          this->Points->GetVoidPointer(0));

  // sort ids according to the x values of the corresponding points
  if (!this->SortedIds)
    {
    this->SortedIds = new vtkIdType[n];
    std::generate(this->SortedIds, this->SortedIds + n, GenSeries());
    std::sort(this->SortedIds, this->SortedIds + n, CompareIds(points));
    }

  // get the lowest point we might hit within the supplied tolerance
  double lowX = point.GetX() - tol.GetX();

  IdToPointXIter begin(points, this->SortedIds);
  IdToPointXIter end(points, this->SortedIds + n);
  IdToPointXIter low = std::lower_bound(begin, end, lowX);

  // set up the array
  if (!this->AreaPointIds)
    {
    this->AreaPointIds = vtkIdTypeArray::New();
    }
  this->AreaPointIds->SetNumberOfTuples(0);

  // now consider the y axis
  float highX = point.GetX() + tol.GetX();
  while (low != end)
    {
    vtkIdType currId = *(low.Pointer);
    if (this->InRange(point, tol, points[currId]))
      {
      this->AreaPointIds->InsertNextValue(currId);
      }
    if (*low > highX)
      {
      break;
      }
    ++low;
    }

  return this->AreaPointIds;
}

// See if the point is within tolerance.
bool vtkVgPlotTimeline::InRange(const vtkVector2f& point,
                                const vtkVector2f& tol,
                                const vtkVector2f& current)
{
  return current.GetX() > point.GetX() - tol.GetX() &&
         current.GetX() < point.GetX() + tol.GetX() &&
         current.GetY() > point.GetY() - tol.GetY() &&
         current.GetY() < point.GetY() + tol.GetY();
}

// See if the point is within tolerance.
bool vtkVgPlotTimeline::InRange(const vtkVector2f& point,
                                const vtkVector2f& tol,
                                const vtkVector2f& p1,
                                const vtkVector2f& p2)
{
  if (p1.GetY() < point.GetY() - tol.GetY() ||
      p1.GetY() > point.GetY() + tol.GetY())
    {
    return false;
    }

  return p1.GetX() < point.GetX() + tol.GetX() &&
         p2.GetX() > point.GetX() - tol.GetX();
}

//-----------------------------------------------------------------------------
void vtkVgPlotTimeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
