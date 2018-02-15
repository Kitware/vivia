/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgRepresentationBase.h"

// VTK includes.
#include <vtkCommand.h>
#include <vtkCellArray.h>
#include <vtkInformation.h>
#include <vtkInformationDoubleKey.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyDataCollection.h>
#include <vtkTransform.h>

vtkInformationKeyMacro(vtkVgRepresentationBase, Z_OFFSET, Double);

//-----------------------------------------------------------------------------
vtkVgRepresentationBase::vtkVgRepresentationBase() : vtkObject(),
  UseAutoUpdate(true),
  LayerIndex(0),
  UpdateRequest(1),
  UseModelMatrix(true),
  ColorMultiplier(1.0),
  HasOverrideColor(false),
  DisplayMask(0)
{
  this->RepresentationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->RepresentationMatrix->Identity();

  this->OffsetRepresentationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->OffsetRepresentationMatrix->Identity();

  this->RenderMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->RenderMatrix->Identity();
}

//-----------------------------------------------------------------------------
vtkVgRepresentationBase::~vtkVgRepresentationBase()
{
}

//-----------------------------------------------------------------------------
void vtkVgRepresentationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVgRepresentationBase::SetRepresentationMatrix(
  const vtkMatrix4x4* matrix)
{
  if (!matrix || (this->RepresentationMatrix == matrix))
    {
    return;
    }

  this->RepresentationMatrix->DeepCopy(matrix);

  // Reset offset as offset is offset wrt representation matrix.
//  this->OffsetRepresentationMatrix->Identity();
//  this->ApplyOffset(this->OffsetRepresentationMatrix);
}

//-----------------------------------------------------------------------------
void vtkVgRepresentationBase::SetLookupTable(vtkLookupTable* lookupTable)
{
  if (!lookupTable || this->LookupTable == lookupTable)
    {
    return;
    }

  this->LookupTable = lookupTable;
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkLookupTable* vtkVgRepresentationBase::GetLookupTable()
{
  return this->LookupTable;
}

//-----------------------------------------------------------------------------
const vtkLookupTable* vtkVgRepresentationBase::GetLookupTable() const
{
  return this->LookupTable;
}

//-----------------------------------------------------------------------------
vtkMatrix4x4* vtkVgRepresentationBase::GetRepresentationMatrix()
{
  return this->RepresentationMatrix;
}

//-----------------------------------------------------------------------------
const vtkMatrix4x4* vtkVgRepresentationBase::GetRepresentationMatrix() const
{
  return this->RepresentationMatrix;
}

//-----------------------------------------------------------------------------
void vtkVgRepresentationBase::ApplyOffset(const vtkMatrix4x4* matrix,
                                          bool pre/*=true*/)
{
  if (!matrix)
    {
    return;
    }

  switch (static_cast<int>(pre))
    {
    case 0:
      {
      vtkMatrix4x4::Multiply4x4(const_cast<vtkMatrix4x4*>(matrix), this->RepresentationMatrix,
                                this->OffsetRepresentationMatrix);
      break;
      }
    case 1:
      {
      vtkMatrix4x4::Multiply4x4(this->RepresentationMatrix, const_cast<vtkMatrix4x4*>(matrix),
                                this->OffsetRepresentationMatrix);
      break;
      }
    }

  // Update needs to be called on this now.
  this->UpdateRequestOn();
}

//-----------------------------------------------------------------------------
void vtkVgRepresentationBase::SetZOffset(vtkProp* prop, double offset)
{
  if (vtkVgRepresentationBase::GetZOffset(prop) == offset)
    {
    return;
    }

  // Get in if test above creates information object if it doesn't exist
  prop->GetPropertyKeys()->Set(Z_OFFSET(), offset);
}

//-----------------------------------------------------------------------------
double vtkVgRepresentationBase::GetZOffset(vtkProp* prop)
{
  vtkInformation* keys = prop->GetPropertyKeys();
  if (!keys)
    {
    keys = vtkInformation::New();
    prop->SetPropertyKeys(keys);
    keys->FastDelete();
    }

  if (!keys->Has(vtkVgRepresentationBase::Z_OFFSET()))
    {
    keys->Set(vtkVgRepresentationBase::Z_OFFSET(), 0.0);
    }

  return keys->Get(vtkVgRepresentationBase::Z_OFFSET());
}

//-----------------------------------------------------------------------------
bool vtkVgRepresentationBase::HasZOffset(vtkProp* prop)
{
  vtkInformation* keys = prop->GetPropertyKeys();
  return keys && keys->Has(vtkVgRepresentationBase::Z_OFFSET());
}

//-----------------------------------------------------------------------------
void vtkVgRepresentationBase::
SetupActorTransform(vtkPolyDataCollection* polyDataCollection,
                    vtkMatrix4x4* originalMatrix,
                    vtkTransform* actorTransform)
{
  // Setup the transform.  Test the RepresentationMatrix against
  // any point in our result (if any), to determine whether or not to
  // multiply through by -1 to handle OpenGL issue with -1 homogeneous
  // coordinate
  vtkMatrix4x4* representationMatrix = originalMatrix;
  vtkSmartPointer<vtkMatrix4x4> correctedMatrix; // if necessary
  if (originalMatrix)
    {
    vtkIdType npts, *pts, thePtId = -1;
    vtkPolyData* polyData;
    polyDataCollection->InitTraversal();
    for (; thePtId == -1 && (polyData = polyDataCollection->GetNextItem());)
      {
      if (polyData->GetLines()->GetNumberOfCells())
        {
        polyData->GetLines()->GetCell(0, npts, pts);
        thePtId = pts[0];
        }
      else if (polyData->GetVerts()->GetNumberOfCells())
        {
        polyData->GetVerts()->GetCell(0, npts, pts);
        thePtId = pts[0];
        }
      }
    if (thePtId != -1)   // we do have data for this rep
      {
      double theTestPoint[4] = {0, 0, 0, 1};
      // if thePtId != -1 (which is the case), then polyData is valid
      polyData->GetPoints()->GetPoint(thePtId, theTestPoint);
      // Check the homogeneous component of the result to see if <0
      if (originalMatrix->MultiplyDoublePoint(theTestPoint)[3] < 0)
        {
        // Multiply by -1 throughout to give homogeneous component> 0;
        // Note, this is guaranteed for this point only, but will usually
        // work for all points
        correctedMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        correctedMatrix->DeepCopy(originalMatrix);
        for (int i = 0; i < 4; i++)
          {
          for (int j = 0; j < 4; j++)
            {
            correctedMatrix->SetElement(i, j, -correctedMatrix->GetElement(i, j));
            }
          }
        representationMatrix = correctedMatrix;
        }
      }
    }

  actorTransform->PostMultiply();  // just to be sure
  actorTransform->SetMatrix(representationMatrix);
}

//-----------------------------------------------------------------------------
void vtkVgRepresentationBase::SetOverrideColor(const double color[3])
{
  if (!color && this->HasOverrideColor)
    {
    this->HasOverrideColor = false;
    this->Modified();
    return;
    }

  if (color && (!this->HasOverrideColor ||
                this->OverrideColor[0] != color[0] ||
                this->OverrideColor[1] != color[1] ||
                this->OverrideColor[2] != color[2]))
    {
    this->HasOverrideColor = true;
    this->OverrideColor[0] = color[0];
    this->OverrideColor[1] = color[1];
    this->OverrideColor[2] = color[2];
    this->Modified();
    }
}
