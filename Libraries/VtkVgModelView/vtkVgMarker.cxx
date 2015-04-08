/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgMarker.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkDistanceToCamera.h>
#include <vtkGlyph3D.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPointSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRegularPolygonSource.h>

vtkStandardNewMacro(vtkVgMarker);

//----------------------------------------------------------------------------
class vtkVgMarker::vtkInternal
{
public:
  // Methods
  vtkInternal(vtkVgMarker* parent);
  ~vtkInternal();

  void UpdatePosition(double* pos);

  // Data memebers
  vtkVgMarker* Parent;

  vtkIdType MarkerPointId;

  vtkSmartPointer<vtkPolyDataMapper> MarkerMapper;
  vtkSmartPointer<vtkPolyData> MarkerPolyData;
  vtkSmartPointer<vtkPoints> MarkerPoints;

  vtkSmartPointer<vtkPointSource> PointSource;
  vtkSmartPointer<vtkGlyph3D> Glyph;

  vtkSmartPointer<vtkRegularPolygonSource> MarkerSource;
  vtkSmartPointer<vtkDistanceToCamera> ScaleFilter;

  double MarkerLocation[3];
};

//--------------------------------------------------------------------------
vtkVgMarker::vtkInternal::vtkInternal(vtkVgMarker* parent) :
  Parent(parent)
{
  this->MarkerMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->MarkerPolyData = vtkSmartPointer<vtkPolyData>::New();

  this->MarkerSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
  this->MarkerSource->SetNumberOfSides(32);
  this->MarkerSource->SetRadius(1);
  this->MarkerSource->SetCenter(0, 0, 0);

  // Create a set of points.
  this->PointSource = vtkSmartPointer<vtkPointSource>::New();
  this->PointSource->SetNumberOfPoints(1);

  // Calculate the distance to the camera of each point.
  this->ScaleFilter = vtkSmartPointer<vtkDistanceToCamera>::New();
  this->ScaleFilter->SetInputConnection(this->PointSource->GetOutputPort());
  this->ScaleFilter->SetScreenSize(10.0);

  this->Glyph = vtkSmartPointer<vtkGlyph3D>::New();
  this->Glyph->SetInputConnection(this->ScaleFilter->GetOutputPort());
  this->Glyph->SetSourceConnection(this->MarkerSource->GetOutputPort());

  // Scale each point.
  this->Glyph->SetScaleModeToScaleByScalar();
  this->Glyph->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "DistanceToCamera");

  this->MarkerMapper->SetInputConnection(this->Glyph->GetOutputPort());
  this->MarkerMapper->SetScalarVisibility(false);

  parent->SetMapper(this->MarkerMapper);
  parent->Deselect();
}

//--------------------------------------------------------------------------
vtkVgMarker::vtkInternal::~vtkInternal()
{
}

//--------------------------------------------------------------------------
void vtkVgMarker::vtkInternal::UpdatePosition(double* pos)
{
  if (pos[0] != Parent->MarkerPosition[0] ||
      pos[1] != Parent->MarkerPosition[1] ||
      pos[2] != Parent->MarkerPosition[2])
    {
    Parent->MarkerPosition[0] = pos[0];
    Parent->MarkerPosition[1] = pos[1];
    Parent->MarkerPosition[2] = pos[2];

    this->PointSource->SetCenter(Parent->MarkerPosition[0],
                                 Parent->MarkerPosition[1],
                                 Parent->MarkerPosition[2]);
    Parent->Modified();
    }
}

//----------------------------------------------------------------------------
vtkVgMarker::vtkVgMarker() :
  vtkOpenGLActor()
{
  this->Id = -1;
  this->Type = -1;

  this->Selected = false;

  this->DefaultColor[0] = 1;
  this->DefaultColor[1] = 1;
  this->DefaultColor[2] = 0;

  this->SelectionColor[0] = 1;
  this->SelectionColor[1] = 0.2;
  this->SelectionColor[2] = 0.2;

  this->MarkerPosition[0] = 0.0;
  this->MarkerPosition[1] = 0.0;
  this->MarkerPosition[2] = 0.0;

  this->Implementation = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkVgMarker::~vtkVgMarker()
{
  delete this->Implementation;
  this->Implementation = NULL;
}

//----------------------------------------------------------------------------
void vtkVgMarker::PrintSelf(ostream& os, vtkIndent indent)
{
}

//----------------------------------------------------------------------------
void vtkVgMarker::SetMarkerPosition(double* pos)
{
  vtkDebugMacro(<< this->GetClassName()
                << " (" << this << "): setting position to " << pos);

  if (!pos)
    {
    vtkWarningMacro(<< "Invalid position for marker");
    }

  this->Implementation->UpdatePosition(pos);
}

//----------------------------------------------------------------------------
void vtkVgMarker::SetRenderer(vtkRenderer* ren)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this
                << "): setting Renderer to " << ren);
  if (this->Implementation->ScaleFilter->GetRenderer() != ren)
    {
    this->Implementation->ScaleFilter->SetRenderer(ren);
    this->Mapper->Update();
    this->Modified();
    }
}

//----------------------------------------------------------------------------
bool vtkVgMarker::IsSelected()
{
  return this->Selected;
}

//----------------------------------------------------------------------------
void vtkVgMarker::Render(vtkRenderer* ren, vtkMapper* mapper)
{
  double* const color =
    (this->Selected ? this->SelectionColor : this->DefaultColor);
  this->GetProperty()->SetColor(color);
  this->Superclass::Render(ren, mapper);
}
