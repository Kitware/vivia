// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqContour.h"

#include <vtkContourWidget.h>
#include <vtkLinearContourLineInterpolator.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkPolyData.h>
#include <vtkPropCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>

#include "vtkVgGeode.h"
#include "vtkVgGroupNode.h"

//-----------------------------------------------------------------------------
vqContour::vqContour(vtkRenderWindowInteractor* iren)
  : Visible(1)
{
  this->ContourWidget = vtkSmartPointer<vtkContourWidget>::New();
  this->ContourWidget->SetInteractor(iren);
  this->Interactor = iren;

  vtkOrientedGlyphContourRepresentation* rep = this->GetRepresentation();
  rep->SetLineColor(1.0, 1.0, 0.0);
  rep->AlwaysOnTopOn();

  vtkLinearContourLineInterpolator* interpolator =
    vtkLinearContourLineInterpolator::New();
  rep->SetLineInterpolator(interpolator);
  interpolator->FastDelete();
}

//-----------------------------------------------------------------------------
vqContour::~vqContour()
{
  if (this->SceneParent)
    {
    this->SceneParent->RemoveChild(this->SceneNode);
    }
}

//-----------------------------------------------------------------------------
void vqContour::Initialize()
{
  this->ContourWidget->On();
  this->ContourWidget->Initialize();
}

//-----------------------------------------------------------------------------
void vqContour::Initialize(vtkPolyData* pd)
{
  this->ContourWidget->On();
  this->ContourWidget->Initialize(pd);
}

//-----------------------------------------------------------------------------
void vqContour::Begin()
{
  this->ContourWidget->FollowCursorOn();
  this->ContourWidget->Render();
}

//-----------------------------------------------------------------------------
void vqContour::End()
{
  // don't auto-add a 'dangling' mouse cursor node
  if (!this->GetRepresentation()->GetClosedLoop())
    {
    this->GetRepresentation()->DeleteLastNode();
    }
  this->ContourWidget->CloseLoop();
  this->DisableInteraction();
}

//-----------------------------------------------------------------------------
void vqContour::Finalize()
{
  this->GetRepresentation()->SetLineColor(1.0, 0.8, 0.0);
}

//-----------------------------------------------------------------------------
void vqContour::AddToScene(vtkVgGroupNode* parent)
{
  this->SceneNode = vtkSmartPointer<vtkVgGeode>::New();
  this->SceneParent = parent;

  vtkPropCollection* pc = vtkPropCollection::New();
  this->GetRepresentation()->GetActors(pc);

  pc->InitTraversal();
  while (vtkProp* prop = pc->GetNextProp())
    {
    this->SceneNode->AddDrawable(prop);
    }

  pc->Delete();

  this->SceneParent->AddChild(this->SceneNode);
  if (!this->Visible)
    {
    this->SceneNode->SetVisible(0);
    }
}

//-----------------------------------------------------------------------------
void vqContour::SetVisible(int visible)
{
  if (visible != this->Visible)
    {
    this->Visible = visible;
    if (this->SceneNode)
      {
      this->SceneNode->SetVisible(visible);
      }
    }
}

//-----------------------------------------------------------------------------
vtkPolyData* vqContour::GetPolyData()
{
  vtkOrientedGlyphContourRepresentation* rep = this->GetRepresentation();
  return rep->GetContourRepresentationAsPolyData();
}

//-----------------------------------------------------------------------------
void vqContour::DisableInteraction()
{
  this->ContourWidget->ProcessEventsOff();

  vtkOrientedGlyphContourRepresentation* rep = this->GetRepresentation();
  rep->GetProperty()->SetOpacity(0.0);
  rep->GetActiveProperty()->SetOpacity(0.0);
}

//-----------------------------------------------------------------------------
vtkOrientedGlyphContourRepresentation* vqContour::GetRepresentation()
{
  vtkOrientedGlyphContourRepresentation* rep =
    vtkOrientedGlyphContourRepresentation::SafeDownCast(
      this->ContourWidget->GetRepresentation());

  return rep;
}
