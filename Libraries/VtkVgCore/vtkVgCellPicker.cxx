/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgCellPicker.h"

// VTK includes.
#include <vtkAbstractCellLocator.h>
#include <vtkAbstractMapper3D.h>
#include <vtkAbstractVolumeMapper.h>
#include <vtkAssemblyPath.h>
#include <vtkBox.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkGenericCell.h>
#include <vtkIdList.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkImageSlice.h>
#include <vtkLODProp3D.h>
#include <vtkMapper.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPerspectiveTransform.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTransform.h>
#include <vtkTexture.h>

vtkStandardNewMacro(vtkVgCellPicker);


//-----------------------------------------------------------------------------
vtkVgCellPicker::vtkVgCellPicker() : vtkCellPicker()
{
  // For polydata picking
  this->Cell = vtkGenericCell::New();
  this->PointIds = vtkIdList::New();

  this->PerspectiveTransform = vtkSmartPointer<vtkPerspectiveTransform>::New();
}

//-----------------------------------------------------------------------------
vtkVgCellPicker::~vtkVgCellPicker()
{
  this->Cell->Delete();
  this->PointIds->Delete();
}

//-----------------------------------------------------------------------------
void vtkVgCellPicker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkVgCellPicker::Pick(double selectionX, double selectionY,
                          double selectionZ, vtkRenderer* renderer)
{
  int i;
  vtkProp* prop;
  vtkCamera* camera;
  vtkAbstractMapper3D* mapper = NULL;
  double p1World[4], p2World[4], p1Mapper[4], p2Mapper[4];
  int winSize[2] = {1, 1};
  double x, y, t;
  double* viewport;
  double cameraPos[4], cameraFP[4];
  double* displayCoords, *worldCoords;
  double* clipRange;
  double ray[3], rayLength;
  int pickable;
  int LODId;
  double windowLowerLeft[4], windowUpperRight[4];
  double bounds[6], tol;
  double tF, tB;
  double hitPosition[3];
  double cameraDOP[3];

  bounds[0] = bounds[1] = bounds[2] = bounds[3] = bounds[4] = bounds[5] = 0;

  //  Initialize picking process
  this->Initialize();
  this->Renderer = renderer;
  this->SelectionPoint[0] = selectionX;
  this->SelectionPoint[1] = selectionY;
  this->SelectionPoint[2] = selectionZ;

  // Invoke start pick method if defined
  this->InvokeEvent(vtkCommand::StartPickEvent, NULL);

  if (renderer == NULL)
    {
    vtkErrorMacro(<< "Must specify renderer!");
    return 0;
    }

  // Get camera focal point and position. Convert to display (screen)
  // coordinates. We need a depth value for z-buffer.
  //
  camera = renderer->GetActiveCamera();
  camera->GetPosition(cameraPos);
  cameraPos[3] = 1.0;
  camera->GetFocalPoint(cameraFP);
  cameraFP[3] = 1.0;

  renderer->SetWorldPoint(cameraFP[0], cameraFP[1], cameraFP[2], cameraFP[3]);
  renderer->WorldToDisplay();
  displayCoords = renderer->GetDisplayPoint();
  selectionZ = displayCoords[2];

  // Convert the selection point into world coordinates.
  //
  renderer->SetDisplayPoint(selectionX, selectionY, selectionZ);
  renderer->DisplayToWorld();
  worldCoords = renderer->GetWorldPoint();
  if (worldCoords[3] == 0.0)
    {
    vtkErrorMacro(<< "Bad homogeneous coordinates");
    return 0;
    }

  for (i = 0; i < 3; i++)
    {
    this->PickPosition[i] = worldCoords[i] / worldCoords[3];
    }

  //  Compute the ray endpoints.  The ray is along the line running from
  //  the camera position to the selection point, starting where this line
  //  intersects the front clipping plane, and terminating where this
  //  line intersects the back clipping plane.
  for (i = 0; i < 3; i++)
    {
    ray[i] = this->PickPosition[i] - cameraPos[i];
    }
  for (i = 0; i < 3; i++)
    {
    cameraDOP[i] = cameraFP[i] - cameraPos[i];
    }

  vtkMath::Normalize(cameraDOP);

  if ((rayLength = vtkMath::Dot(cameraDOP, ray)) == 0.0)
    {
    vtkWarningMacro("Cannot process points");
    return 0;
    }

  clipRange = camera->GetClippingRange();

  if (camera->GetParallelProjection())
    {
    tF = clipRange[0] - rayLength;
    tB = clipRange[1] - rayLength;
    for (i = 0; i < 3; i++)
      {
      p1World[i] = this->PickPosition[i] + tF * cameraDOP[i];
      p2World[i] = this->PickPosition[i] + tB * cameraDOP[i];
      }
    }
  else
    {
    tF = clipRange[0] / rayLength;
    tB = clipRange[1] / rayLength;
    for (i = 0; i < 3; i++)
      {
      p1World[i] = cameraPos[i] + tF * ray[i];
      p2World[i] = cameraPos[i] + tB * ray[i];
      }
    }
  p1World[3] = p2World[3] = 1.0;

  // Compute the tolerance in world coordinates.  Do this by
  // determining the world coordinates of the diagonal points of the
  // window, computing the width of the window in world coordinates, and
  // multiplying by the tolerance.
  //
  viewport = renderer->GetViewport();
  if (renderer->GetRenderWindow())
    {
    int* winSizePtr = renderer->GetRenderWindow()->GetSize();
    if (winSizePtr)
      {
      winSize[0] = winSizePtr[0];
      winSize[1] = winSizePtr[1];
      }
    }
  x = winSize[0] * viewport[0];
  y = winSize[1] * viewport[1];
  renderer->SetDisplayPoint(x, y, selectionZ);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(windowLowerLeft);

  x = winSize[0] * viewport[2];
  y = winSize[1] * viewport[3];
  renderer->SetDisplayPoint(x, y, selectionZ);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(windowUpperRight);

  for (tol = 0.0, i = 0; i < 3; i++)
    {
    tol += (windowUpperRight[i] - windowLowerLeft[i]) *
           (windowUpperRight[i] - windowLowerLeft[i]);
    }

  tol = sqrt(tol) * this->Tolerance;

  //  Loop over all props.  Transform ray (defined from position of
  //  camera to selection point) into coordinates of mapper (not
  //  transformed to actors coordinates!  Reduces overall computation!!!).
  //  Note that only vtkProp3D's can be picked by vtkPicker.
  //
  vtkPropCollection* props;
  vtkProp* propCandidate;
  if (this->PickFromList)
    {
    props = this->GetPickList();
    }
  else
    {
    props = renderer->GetViewProps();
    }

  vtkActor* actor;
  vtkLODProp3D* prop3D;
  vtkVolume* volume;
  vtkImageSlice* imageSlice = 0;
  vtkAssemblyPath* path;
  vtkProperty* tempProperty;
  this->Transform->PostMultiply();
  vtkCollectionSimpleIterator pit;
  double scale[3];
  for (props->InitTraversal(pit); (prop = props->GetNextProp(pit));)
    {
    for (prop->InitPathTraversal(); (path = prop->GetNextPath());)
      {
      pickable = 0;
      actor = NULL;
      propCandidate = path->GetLastNode()->GetViewProp();
      if (propCandidate->GetPickable() && propCandidate->GetVisibility())
        {
        pickable = 1;
        if ((actor = vtkActor::SafeDownCast(propCandidate)) != NULL)
          {
          mapper = actor->GetMapper();
          if (actor->GetProperty()->GetOpacity() <= 0.0)
            {
            pickable = 0;
            }
          }
        else if ((prop3D = vtkLODProp3D::SafeDownCast(propCandidate)) != NULL)
          {
          LODId = prop3D->GetPickLODID();
          mapper = prop3D->GetLODMapper(LODId);

          // if the mapper is a vtkMapper (as opposed to a vtkVolumeMapper),
          // then check the transparency to see if the object is pickable
          if (vtkMapper::SafeDownCast(mapper) != NULL)
            {
            prop3D->GetLODProperty(LODId, &tempProperty);
            if (tempProperty->GetOpacity() <= 0.0)
              {
              pickable = 0;
              }
            }
          }
        else if ((volume = vtkVolume::SafeDownCast(propCandidate)) != NULL)
          {
          mapper = volume->GetMapper();
          }
        else if ((imageSlice = vtkImageSlice::SafeDownCast(propCandidate)))
          {
          mapper = imageSlice->GetMapper();
          }
        else
          {
          pickable = 0; //only vtkProp3D's (actors and volumes) can be picked
          }
        }

      //  If actor can be picked, get its composite matrix, invert it, and
      //  use the inverted matrix to transform the ray points into mapper
      //  coordinates.
      if (pickable)
        {
        vtkMatrix4x4* lastMatrix = path->GetLastNode()->GetMatrix();
        if (lastMatrix == NULL)
          {
          vtkErrorMacro(<< "Pick: Null matrix.");
          return 0;
          }

        this->Transform->SetMatrix(lastMatrix);
        this->Transform->Push();
        this->Transform->Inverse();
        this->Transform->GetScale(scale); //need to scale the tolerance


        this->PerspectiveTransform->SetMatrix(lastMatrix);
        this->PerspectiveTransform->Inverse();
        this->PerspectiveTransform->TransformPoint(p1World, p1Mapper);
        this->PerspectiveTransform->TransformPoint(p2World, p2Mapper);

        for (i = 0; i < 3; i++)
          {
          ray[i] = p2Mapper[i] - p1Mapper[i];
          }

        this->Transform->Pop();

        //  Have the ray endpoints in mapper space, now need to compare this
        //  with the mapper bounds to see whether intersection is possible.
        //
        //  Get the bounding box of the modeller.  Note that the tolerance is
        //  added to the bounding box to make sure things on the edge of the
        //  bounding box are picked correctly.
        if (mapper != NULL)
          {
          mapper->GetBounds(bounds);
          }

        bounds[0] -= tol; bounds[1] += tol;
        bounds[2] -= tol; bounds[3] += tol;
        bounds[4] -= tol; bounds[5] += tol;

        if (vtkBox::IntersectBox(bounds, p1Mapper, ray, hitPosition, t))
          {
          t = this->IntersectWithLine(
                p1Mapper, p2Mapper, tol * 0.333 * (scale[0] + scale[1] + scale[2]),
                path, static_cast<vtkProp3D*>(propCandidate), mapper);

          if (t < VTK_DOUBLE_MAX)
            {
            double p[3];
            p[0] = (1.0 - t) * p1World[0] + t * p2World[0];
            p[1] = (1.0 - t) * p1World[1] + t * p2World[1];
            p[2] = (1.0 - t) * p1World[2] + t * p2World[2];

            // The IsItemPresent method returns "index+1"
            int prevIndex = this->Prop3Ds->IsItemPresent(prop) - 1;

            if (prevIndex >= 0)
              {
              // If already in list, set point to the closest point
              double oldp[3];
              this->PickedPositions->GetPoint(prevIndex, oldp);
              if (vtkMath::Distance2BetweenPoints(p1World, p) <
                  vtkMath::Distance2BetweenPoints(p1World, oldp))
                {
                this->PickedPositions->SetPoint(prevIndex, p);
                }
              }
            else
              {
              this->Prop3Ds->AddItem(static_cast<vtkProp3D*>(prop));

              this->PickedPositions->InsertNextPoint(p);

              // backwards compatibility: also add to this->Actors
              if (actor)
                {
                this->Actors->AddItem(actor);
                }
              }
            }
          }
        }//if visible and pickable and not transparent
      }//for all parts
    }//for all actors

  int picked = 0;

  if (this->Path)
    {
    // Invoke pick method if one defined - prop goes first
    this->Path->GetFirstNode()->GetViewProp()->Pick();
    this->InvokeEvent(vtkCommand::PickEvent, NULL);
    picked = 1;
    }

  // Invoke end pick method if defined
  this->InvokeEvent(vtkCommand::EndPickEvent, NULL);

  return picked;
}

//----------------------------------------------------------------------------
double vtkVgCellPicker::IntersectActorWithLine(const double p1[3],
                                               const double p2[3],
                                               double t1, double t2,
                                               double tol,
                                               vtkProp3D* prop,
                                               vtkMapper* mapper)
{
  vtkDataSet* data = mapper->GetInput();
  double* bounds = data->GetBounds();

  vtkCollectionSimpleIterator iter;
  vtkAbstractCellLocator* locator = 0;
  this->Locators->InitTraversal(iter);
  while ((locator = static_cast<vtkAbstractCellLocator*>(
                      this->Locators->GetNextItemAsObject(iter))))
    {
    if (locator->GetDataSet() == data)
      {
      break;
      }
    }

  // If we have a locator, then use the original picking functionality.  Also,
  // to use this "special" picking capability the data needs to be 2D, and for
  // now that means z is constant.  Similarly, we are assuming, for now, that
  // the pick ray is perpendicular to the z plane (thus p1==p2 except for z).
  if (locator || bounds[4] != bounds[5] ||
      p1[0] != p2[0] || p1[1] != p2[1] || p1[2] == p2[2])
    {
    return this->Superclass::IntersectActorWithLine(p1, p2, t1, t2,
                                                    tol, prop, mapper);
    }

  // Intersect our pick ray with the 2D plane to come up with the pt to
  // compare to cells in the polydata; need tMin to be parametric coordinate
  // on our intersecting ray
  double tMin = (bounds[4] - p2[2]) / (p1[2] - p2[2]);

  // initially will assume ray perpendicular thus can just use x, y components
  double planeIntersectionPt[3] = { p1[0], p1[1], bounds[4] };

  double minDist2 = VTK_DOUBLE_MAX;
  double backupMinDist2 = VTK_DOUBLE_MAX;
  double minPCoords[3] = { 0.0, 0.0, 0.0 };
  vtkIdType minCellId = -1;
  int minSubId = -1;
  double minXYZ[3] = { 0.0, 0.0, 0.0 };

  // If pick is inside a cell, want to find inner most cell that is picked
  double minInsideDistance = VTK_DOUBLE_MAX;
  vtkIdType minInsideCellId = -1;
  int minInsideSubId = -1;

  vtkIdList* pointIds = this->PointIds;
  vtkIdType numCells = data->GetNumberOfCells();
  double* weights = new double [data->GetMaxCellSize()];
  double* minWeights = new double [data->GetMaxCellSize()];
  double cellNormal[3] = { 0, 0, 1.0 };

  for (vtkIdType cellId = 0; cellId < numCells && minDist2 > 0; ++cellId)
    {
    double pcoords[3], closestPt[3], dist2;
    int newSubId, numSubIds = 1;
    bool pickedInsideClosedPolyline = false;

    // If it is a strip, we need to iterate over the subIds
    int cellType = data->GetCellType(cellId);
    int useSubCells = this->HasSubCells(cellType);
    if (useSubCells)
      {
      // Get the pointIds for the strip and the length of the strip
      data->GetCellPoints(cellId, pointIds);
      numSubIds = this->GetNumberOfSubCells(pointIds, cellType);

      // If a closed polyline, test to see if picked "inside" the cell
      // Note: for a polyline, numSubIds = NumberOfPoints - 1
      if (cellType == VTK_POLY_LINE &&
          pointIds->GetId(0) == pointIds->GetId(numSubIds))
        {
        data->GetCell(cellId, this->Cell);
        vtkDoubleArray* cellPoints =
          static_cast<vtkDoubleArray*>(this->Cell->GetPoints()->GetData());
        if (vtkPolygon::PointInPolygon(planeIntersectionPt,
                                       numSubIds, cellPoints->GetPointer(0),
                                       this->Cell->GetBounds(), cellNormal))
          {
          pickedInsideClosedPolyline = true;
          // Reset the min distance to make sure we're finding the closest
          // subcell from this cell (but back it up first, as we may need to
          // restore it)
          backupMinDist2 = minDist2;
          minDist2 = VTK_DOUBLE_MAX;
          }
        }
      }

    // This will only loop once unless we need to deal with a strip
    for (vtkIdType subId = 0; subId < numSubIds; ++subId)
      {
      if (useSubCells)
        {
        // Get a sub-cell from the strip
        this->GetSubCell(data, pointIds, subId, cellType, this->Cell);
        }
      else
        {
        data->GetCell(cellId, this->Cell);
        }

      // Evaluate the position of the point relative to the cell (subcell).
      // Note: we really only care about the distance, but this is the common
      // method within vtkGenericCell that returns the dist2
      if (this->Cell->EvaluatePosition(planeIntersectionPt, closestPt,
                                       newSubId, pcoords, dist2, weights) != -1)
        {
        if (dist2 < minDist2)
          {
          minDist2 = dist2;
          // save all of these
          minCellId = cellId;
          minSubId = newSubId;
          if (useSubCells)
            {
            minSubId = subId;
            }
          for (int k = 0; k < 3; k++)
            {
            minXYZ[k] = closestPt[k];
            minPCoords[k] = pcoords[k];
            }
          vtkIdType numPoints = this->Cell->GetNumberOfPoints();
          for (vtkIdType i = 0; i < numPoints; ++i)
            {
            minWeights[i] = weights[i];
            }
          }
        }
      }
    if (pickedInsideClosedPolyline)
      {
      if (minDist2 < minInsideDistance)
        {
        minInsideDistance = minDist2;
        minInsideCellId = minCellId;
        minInsideSubId = minSubId;
        }
      else
        {
        // Very unlikely case that we determined we picked inside closed
        // polyline but didn't succcessfuly EvaluatePosition for ANY of the sub
        // cells; reset the min distance to our previous best
        minDist2 = backupMinDist2;
        }
      }
    }

  // If we successfully detected a pick inside a cell, we want to use
  // that as out picked cell/subcell
  if (minInsideDistance < VTK_DOUBLE_MAX)
    {
    minDist2 = 0;  // inside the polyline, so an exact pick
    minCellId = minInsideCellId;
    minSubId = minInsideSubId;
    }

  // Did we find a cell that meets the tolerance requirements?  If so, we need
  // to do some things with the cell.
  if (minDist2 < tol * tol && minCellId >= 0 && tMin < this->GlobalTMin)
    {
    this->ResetPickInfo();

    // Get the cell, convert to triangle if it is a strip
    vtkGenericCell* cell = this->Cell;

    // get the picked cell
    int cellType = data->GetCellType(minCellId);
    if (this->HasSubCells(cellType))
      {
      data->GetCellPoints(minCellId, this->PointIds);
      this->GetSubCell(data, this->PointIds, minSubId, cellType, cell);
      }
    else
      {
      data->GetCell(minCellId, cell);
      }

    vtkIdType numPoints = cell->GetNumberOfPoints();
    this->Mapper = mapper;

    // Get the texture from the actor or the LOD
    vtkActor* actor = 0;
    vtkLODProp3D* lodActor = 0;
    if ((actor = vtkActor::SafeDownCast(prop)))
      {
      this->Texture = actor->GetTexture();
      }
    else if ((lodActor = vtkLODProp3D::SafeDownCast(prop)))
      {
      int lodId = lodActor->GetPickLODID();
      lodActor->GetLODTexture(lodId, &this->Texture);
      }

    if (this->PickTextureData && this->Texture)
      {
      // Return the texture's image data to the user
      vtkImageData* image = this->Texture->GetInput();
      this->DataSet = image;

      // Get and check the image dimensions
      int extent[6];
      image->GetExtent(extent);
      int dimensionsAreValid = 1;
      int dimensions[3];
      for (int i = 0; i < 3; i++)
        {
        dimensions[i] = extent[2 * i + 1] - extent[2 * i] + 1;
        dimensionsAreValid = (dimensionsAreValid && dimensions[i] > 0);
        }

      // Use the texture coord to set the information
      double tcoord[3];
      if (dimensionsAreValid &&
          this->ComputeSurfaceTCoord(data, cell, minWeights, tcoord))
        {
        // Take the border into account when computing coordinates
        double x[3];
        x[0] = extent[0] + tcoord[0] * dimensions[0] - 0.5;
        x[1] = extent[2] + tcoord[1] * dimensions[1] - 0.5;
        x[2] = extent[4] + tcoord[2] * dimensions[2] - 0.5;

        this->SetImageDataPickInfo(x, extent);
        }
      }
    else
      {
      // Return the polydata to the user
      this->DataSet = data;
      this->CellId = minCellId;
      this->SubId = minSubId;
      this->PCoords[0] = minPCoords[0];
      this->PCoords[1] = minPCoords[1];
      this->PCoords[2] = minPCoords[2];

      // Find the point with the maximum weight
      double maxWeight = 0;
      vtkIdType iMaxWeight = -1;
      for (vtkIdType i = 0; i < numPoints; i++)
        {
        if (minWeights[i] > maxWeight)
          {
          iMaxWeight = i;
          }
        }

      // If maximum weight is found, use it to get the PointId
      if (iMaxWeight != -1)
        {
        this->PointId = cell->PointIds->GetId(iMaxWeight);
        }
      }

    // Set the mapper position
    this->MapperPosition[0] = minXYZ[0];
    this->MapperPosition[1] = minXYZ[1];
    this->MapperPosition[2] = minXYZ[2];

    // Compute the normal
    if (!this->ComputeSurfaceNormal(data, cell, minWeights, this->MapperNormal))
      {
      // By default, the normal points back along view ray
      this->MapperNormal[0] = p1[0] - p2[0];
      this->MapperNormal[1] = p1[1] - p2[1];
      this->MapperNormal[2] = p1[2] - p2[2];
      vtkMath::Normalize(this->MapperNormal);
      }
    }
  else
    {
    tMin = VTK_DOUBLE_MAX;  // must return large tMin if intersection not found
    }

  delete [] weights;
  delete [] minWeights;

  // return value is parametric coordinate along pick ray where hit occurred
  return tMin;
}



