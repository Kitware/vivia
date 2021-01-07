// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgSelectionListRepresentation.h"
#include "vtkRenderer.h"
#include "vtkPlaneSource.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkPolyData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkActor2D.h"
#include "vtkProperty2D.h"
#include "vtkLookupTable.h"
#include "vtkBalloonRepresentation.h"
#include "vtkWindow.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkCamera.h"
#include <set>
#include <map>
#include <utility>

vtkStandardNewMacro(vtkVgSelectionListRepresentation);
vtkCxxSetObjectMacro(vtkVgSelectionListRepresentation, LookupTable, vtkScalarsToColors);

//-- Define the PIMPLd array defining the selection information
struct vtkVgSelectionBalloon
{
  vtkIdType     ItemID;
  double        Priority;
  vtkStdString  Text;
  vtkImageData* Image;

  vtkVgSelectionBalloon() : ItemID(-1), Priority(1.0), Text(), Image(0) {}
  vtkVgSelectionBalloon(vtkIdType itemID, double p, vtkStdString* str, vtkImageData* img)
    {
    this->ItemID = itemID;
    this->Priority = p;
    this->Text = *str;
    this->Image = img;
    }
  vtkVgSelectionBalloon(vtkIdType itemID, double p, const char* str, vtkImageData* img)
    {
    this->ItemID = itemID;
    this->Priority = p;
    this->Text = vtkStdString(str);
    this->Image = img;
    }
  ~vtkVgSelectionBalloon()
    {
    }
  void operator=(const vtkVgSelectionBalloon& balloon)
    {
    this->ItemID = balloon.ItemID;
    this->Priority = balloon.Priority;
    this->Text = balloon.Text;
    this->Image = balloon.Image;
    }
  int operator==(const vtkVgSelectionBalloon& balloon) const
    {
    if (this->ItemID == balloon.ItemID)
      {
      if (this->Priority == balloon.Priority)
        {
        if (this->Image == balloon.Image)
          {
          if (this->Text == balloon.Text)
            {
            return 1;
            }
          }
        }
      }
    return 0;
    }
  int operator!=(const vtkVgSelectionBalloon& balloon) const
    {
    return !(*this == balloon);
    }
};

// Comparison structure for set
struct CompareBalloon
{
  bool operator()(vtkVgSelectionBalloon b1, vtkVgSelectionBalloon b2)
    {
    if (b1.Priority == b2.Priority)
      {
      return b1.ItemID > b2.ItemID;
      }
    else
      {
      return b1.Priority > b2.Priority;
      }
    }
};

class vtkVgSelectionSet :
  public std::set<vtkVgSelectionBalloon, CompareBalloon>
{
};

typedef std::set<vtkVgSelectionBalloon, CompareBalloon>::iterator
  vtkVgSelectionSetIterator;
typedef std::pair<vtkVgSelectionSetIterator, bool> vtkVgSelectionSetRValue;

class vtkVgSelectionMap :
  public std::map<vtkIdType, vtkVgSelectionSetIterator>
{
};

typedef std::map<vtkIdType, vtkVgSelectionSetIterator>::iterator
  vtkVgSelectionMapIterator;

//-------------------------------------------------------------------------
vtkVgSelectionListRepresentation::vtkVgSelectionListRepresentation()
{
  this->InteractionState = vtkVgSelectionListRepresentation::Outside;

  // Initial positioning information
  this->PositionCoordinate = vtkCoordinate::New();
  this->PositionCoordinate->SetCoordinateSystemToWorld();
  this->PositionCoordinate->SetValue(0, 0, 0);
  this->Position2Coordinate = vtkCoordinate::New();
  this->Position2Coordinate->SetCoordinateSystemToDisplay();
  this->Position2Coordinate->SetValue(25, 100);
  this->Position2Coordinate->SetReferenceCoordinate(this->PositionCoordinate);

  // Initial lookup table is standard rainbow
  this->LookupTable = vtkLookupTable::New();
  this->LookupTable->Build();

  // Control appearance
  this->Orientation = VerticalList;

  // Create the geometry in canonical coordinates
  this->Buttons = vtkPlaneSource::New();
  this->ColoredButtons = vtkPolyData::New();
  this->Ids = vtkIdTypeArray::New();
  this->ButtonMapper = vtkPolyDataMapper2D::New();
  this->ButtonMapper->SetInputData(this->ColoredButtons);
  this->ButtonActor = vtkActor2D::New();
  this->ButtonActor->SetMapper(this->ButtonMapper);

  // Internal state
  this->SelectionSet = new vtkVgSelectionSet;
  this->SelectionMap = new vtkVgSelectionMap;
  this->CurrentLocation = -1;
  this->CurrentItemId = -1;
  this->SelectedLocation = -1;
  this->SelectedItemId = -1;

  // Create the balloon
  this->Balloon = vtkBalloonRepresentation::New();
  this->Balloon->SetBalloonLayoutToImageRight();
  this->Balloon->SetPadding(1);
  this->BalloonOffset = 5;

  // Create the selection marker
  vtkCellArray* markerPolys = vtkCellArray::New();
  markerPolys->InsertNextCell(3);
  markerPolys->InsertCellPoint(0);
  markerPolys->InsertCellPoint(1);
  markerPolys->InsertCellPoint(2);
  markerPolys->InsertNextCell(3);
  markerPolys->InsertCellPoint(3);
  markerPolys->InsertCellPoint(4);
  markerPolys->InsertCellPoint(5);
  this->Marker = vtkPolyData::New();
  this->MarkerPoints = vtkPoints::New();
  this->MarkerPoints->SetNumberOfPoints(6);
  this->Marker->SetPoints(this->MarkerPoints);
  this->Marker->SetPolys(markerPolys);
  markerPolys->Delete();
  this->MarkerMapper = vtkPolyDataMapper2D::New();
  this->MarkerMapper->SetInputData(this->Marker);
  this->MarkerActor = vtkActor2D::New();
  this->MarkerActor->SetMapper(this->MarkerMapper);
}

//-------------------------------------------------------------------------
vtkVgSelectionListRepresentation::~vtkVgSelectionListRepresentation()
{
  this->PositionCoordinate->Delete();
  this->Position2Coordinate->Delete();

  delete this->SelectionSet;
  delete this->SelectionMap;

  this->SetLookupTable(0);
  this->Balloon->Delete();
  this->Buttons->Delete();
  this->ColoredButtons->Delete();
  this->Ids->Delete();
  this->ButtonMapper->Delete();
  this->ButtonActor->Delete();

  this->Marker->Delete();
  this->MarkerPoints->Delete();
  this->MarkerMapper->Delete();
  this->MarkerActor->Delete();
}

//-------------------------------------------------------------------------
int vtkVgSelectionListRepresentation::
ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
{
  vtkIdType currentId = this->CurrentItemId;
  int state = this->InteractionState;

  // Make sure something has been defined
  int numItems = this->GetNumberOfItems();
  if (numItems < 1)
    {
    this->InteractionState = vtkVgSelectionListRepresentation::Outside;
    }
  else // do more work
    {
    // Perform geometric query
    int* pos1 = this->PositionCoordinate->
                GetComputedDisplayValue(this->Renderer);
    int* pos2 = this->Position2Coordinate->
                GetComputedDisplayValue(this->Renderer);

    // Figure out where we are in the widget. Exclude outside case first.
    if (X < pos1[0] || pos2[0] < X || Y < pos1[1] || pos2[1] < Y)
      {
      this->InteractionState = vtkVgSelectionRepresentation::Outside;
      }
    else // we are on the boundary or inside the border
      {
      if (this->Orientation == HorizontalList)
        {
        double spacing = static_cast<double>(pos2[0] - pos1[0]) / numItems;
        this->CurrentLocation = static_cast<int>(X - pos1[0]) / spacing;
        }
      else //VerticalList
        {
        double spacing = static_cast<double>(pos2[1] - pos1[1]) / numItems;
        this->CurrentLocation = static_cast<int>(Y - pos1[1]) / spacing;
        }
      this->CurrentItemId = this->Ids->GetValue(this->CurrentLocation);
      this->CurrentLocation = (numItems - 1) - this->CurrentLocation;
      this->InteractionState = vtkVgSelectionRepresentation::OnItem;
      }

    if (state != this->InteractionState ||
        currentId != this->CurrentItemId)
      {
      this->Modified();
      }
    }

  return this->InteractionState;
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::SelectCurrentItem()
{
  int numItems = this->GetNumberOfItems();
  if (this->CurrentItemId < 0 || numItems < 1)
    {
    this->MarkerActor->VisibilityOff();
    this->SelectedLocation = -1;
    this->SelectedItemId = -1;
    }
  else
    {
    this->SelectedLocation = this->CurrentLocation;
    this->SelectedItemId = this->CurrentItemId;
    this->MarkerActor->VisibilityOn();
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::DrawSelectionMarker()
{
  int numItems = this->GetNumberOfItems();
  if (this->SelectedLocation < 0 || this->SelectedLocation > (numItems - 1))
    {
    this->MarkerActor->VisibilityOff();
    return;
    }

  // Perform geometric query
  int* pos1 = this->PositionCoordinate->
              GetComputedDisplayValue(this->Renderer);
  int* pos2 = this->Position2Coordinate->
              GetComputedDisplayValue(this->Renderer);

  double sx, sy, xOffset, yOffset, o[3];
  double tp0[3], tp1[3], tp2[3], tp3[3], tp4[3], tp5[3];
  tp0[2] = tp1[2] = tp2[2] = tp3[2] = tp4[2] = tp5[2] = 0.0;
  o[0] = pos1[0]; o[1] = pos1[1]; o[2] = 0.0;
  if (this->Orientation == HorizontalList)
    {
    sx = static_cast<double>(pos2[0] - pos1[0]) / numItems;
    sy = static_cast<double>(pos2[0] - pos1[0]);
    xOffset = o[0] + ((numItems - 1) - this->SelectedLocation) * sx + sx / 2;
    yOffset = o[1] + sy / 2;
    tp0[0] = xOffset;
    tp0[1] = yOffset + sy / 2;
    tp1[0] = xOffset + sx / 2;
    tp1[1] = yOffset + sy;
    tp2[0] = xOffset - sx / 2;
    tp2[1] = yOffset + sy;
    tp3[0] = xOffset;
    tp3[1] = yOffset - sy / 2;
    tp4[0] = xOffset + sx / 2;
    tp4[1] = yOffset - sy;
    tp5[0] = xOffset - sx / 2;
    tp5[1] = yOffset - sy;
    }
  else //VerticalList
    {
    sx = static_cast<double>(pos2[0] - pos1[0]);
    sy = static_cast<double>(pos2[1] - pos1[1]) / numItems;
    xOffset = o[0] + sx / 2;
    yOffset = o[1] + ((numItems - 1) - this->SelectedLocation) * sy + sy / 2;
    tp0[0] = xOffset - sx / 2;
    tp0[1] = yOffset;
    tp1[0] = xOffset - sx;
    tp1[1] = yOffset + sy / 2;
    tp2[0] = xOffset - sx;
    tp2[1] = yOffset - sy / 2;
    tp3[0] = xOffset + sx / 2;
    tp3[1] = yOffset;
    tp4[0] = xOffset + sx;
    tp4[1] = yOffset - sy / 2;
    tp5[0] = xOffset + sx;
    tp5[1] = yOffset + sy / 2;
    }
  this->MarkerPoints->SetPoint(0, tp0);
  this->MarkerPoints->SetPoint(1, tp1);
  this->MarkerPoints->SetPoint(2, tp2);
  this->MarkerPoints->SetPoint(3, tp3);
  this->MarkerPoints->SetPoint(4, tp4);
  this->MarkerPoints->SetPoint(5, tp5);
  this->MarkerPoints->Modified();
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::UnselectCurrentItem()
{
  this->MarkerActor->VisibilityOff();
  this->SelectedLocation = -1;
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::SetSelectedItemId(vtkIdType id)
{
  vtkVgSelectionMapIterator miter = this->SelectionMap->find(id);
  if (miter != this->SelectionMap->end())
    {
    this->SelectedItemId = (*miter->second).ItemID;
    int numItems = this->GetNumberOfItems();
    vtkVgSelectionSetIterator iter = this->SelectionSet->begin();
    for (int i = 0; i < numItems; ++i)
      {
      if ((*iter).ItemID == id)
        {
        this->SelectedLocation = (numItems - 1) - i;
        break;
        }
      ++iter;
      }
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::SetSelectedLocation(int loc)
{
  int numItems = this->GetNumberOfItems();
  if (loc < 0 || loc >= numItems)
    {
    return;
    }

  this->SelectedLocation = loc;
  vtkVgSelectionSetIterator iter = this->SelectionSet->begin();
  for (int i = 0; i < numItems; ++i)
    {
    if ((numItems - 1 - i) == this->SelectedLocation)
      {
      this->SelectedItemId = (*iter).ItemID;
      break;
      }
    ++iter;
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::BuildRepresentation()
{
  // Nothing appears id no items defined
  int numItems = this->GetNumberOfItems();
  if (numItems < 1)
    {
    return;
    }

  this->Balloon->SetRenderer(this->Renderer);

  if (this->GetMTime() > this->BuildTime ||
      (this->Renderer && this->Renderer->GetActiveCamera() &&
       this->Renderer->GetActiveCamera()->GetMTime() > this->BuildTime) ||
      (this->Renderer && this->Renderer->GetVTKWindow() &&
       this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime))
    {
    // Set things up
    int* pos1 = this->PositionCoordinate->
                GetComputedDisplayValue(this->Renderer);
    int* pos2 = this->Position2Coordinate->
                GetComputedDisplayValue(this->Renderer);

    // Now transform the canonical widget into display coordinates
    double size[2];
    this->GetSize(size);
    double sx = (pos2[0] - pos1[0]) / size[0];
    double sy = (pos2[1] - pos1[1]) / size[1];

    // Generate the buttons.
    if (this->Orientation == VerticalList)
      {
      this->Buttons->SetResolution(1, numItems);
      }
    else
      {
      this->Buttons->SetResolution(numItems, 1);
      }

    double o[3], p1[3], p2[3];
    o[0] = pos1[0]; o[1] = pos1[1]; o[2] = 0.0;
    p1[0] = pos1[0] + sx; p1[1] = pos1[1]; p1[2] = 0.0;
    p2[0] = pos1[0]; p2[1] = pos1[1] + sy; p2[2] = 0.0;
    this->Buttons->SetOrigin(o);
    this->Buttons->SetPoint1(p1);
    this->Buttons->SetPoint2(p2);
    this->Buttons->Update();
    this->ColoredButtons->ShallowCopy(this->Buttons->GetOutput());

    // Now create the scalars
    this->ButtonMapper->SetLookupTable(this->LookupTable);
    this->ButtonMapper->UseLookupTableScalarRangeOn();
    vtkDoubleArray* scalars = vtkDoubleArray::New();
    scalars->SetNumberOfTuples(numItems);
    this->Ids->SetNumberOfTuples(numItems);
    vtkVgSelectionSetIterator iter = this->SelectionSet->begin();
    for (int i = 0; i < numItems; ++i)
      {
      this->Ids->SetTuple1(i, (*iter).ItemID);
      scalars->SetTuple1(i, (*iter).Priority);
      ++iter;
      }
    this->ColoredButtons->GetCellData()->SetScalars(scalars);
    scalars->Delete();

    // Bring up the balloon if valid
    this->Balloon->VisibilityOff();
    if (this->AnnotationVisibility &&
        this->InteractionState == vtkVgSelectionRepresentation::OnItem)
      {
      vtkVgSelectionMapIterator miter = this->SelectionMap->find(this->CurrentItemId);
      if (miter != this->SelectionMap->end())
        {
        this->AnnotationVisibilityOn();
        iter = miter->second;
        this->Balloon->VisibilityOn();
        this->Balloon->SetRenderer(this->Renderer);
        this->Balloon->SetBalloonText(const_cast<vtkStdString&>((*iter).Text));
        this->Balloon->SetBalloonImage((*iter).Image);
        // Compute offset
        double spacing, xOffset, yOffset;
        if (this->Orientation == HorizontalList)
          {
          spacing = static_cast<double>(pos2[0] - pos1[0]) / numItems;
          xOffset = o[0] + ((numItems - 1) - this->CurrentLocation) * spacing;
          yOffset = o[1] + sy + this->BalloonOffset;
          }
        else //VerticalList
          {
          spacing = static_cast<double>(pos2[1] - pos1[1]) / numItems;
          xOffset = o[0] + sx + this->BalloonOffset;
          yOffset = o[1] + ((numItems - 1) - this->CurrentLocation) * spacing;
          }

        this->Balloon->SetOffset(xOffset, yOffset);
        }
      }

    // Draw marker if something selected
    if (this->MarkerActor->GetVisibility())
      {
      this->DrawSelectionMarker();
      }

    this->BuildTime.Modified();
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::GetActors2D(vtkPropCollection* pc)
{
  pc->AddItem(this->ButtonActor);
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::ReleaseGraphicsResources(vtkWindow* w)
{
  this->ButtonActor->ReleaseGraphicsResources(w);
}

//-------------------------------------------------------------------------
int vtkVgSelectionListRepresentation::RenderOverlay(vtkViewport* w)
{
  // Nothing appears id no items defined
  if (this->GetNumberOfItems() < 1)
    {
    return 0;
    }

  if (! this->ButtonActor->GetVisibility())
    {
    return 0;
    }

  this->BuildRepresentation();

  int count = 0;
  count += this->ButtonActor->RenderOverlay(w);
  if (this->AnnotationVisibility)
    {
    count += this->Balloon->RenderOverlay(w);
    }
  if (this->SelectedLocation >= 0)
    {
    count += this->MarkerActor->RenderOverlay(w);
    }

  return count;
}

//-------------------------------------------------------------------------
int vtkVgSelectionListRepresentation::RenderOpaqueGeometry(vtkViewport* w)
{
  // Nothing appears id no items defined
  if (this->GetNumberOfItems() < 1)
    {
    return 0;
    }

  if (! this->ButtonActor->GetVisibility())
    {
    return 0;
    }
  this->BuildRepresentation();

  int count = 0;
  count += this->ButtonActor->RenderOpaqueGeometry(w);
  if (this->AnnotationVisibility)
    {
    count += this->Balloon->RenderOpaqueGeometry(w);
    }
  if (this->SelectedLocation >= 0)
    {
    count += this->MarkerActor->RenderOpaqueGeometry(w);
    }

  return count;
}

//-----------------------------------------------------------------------------
int vtkVgSelectionListRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* w)
{
  // Nothing appears id no items defined
  if (this->GetNumberOfItems() < 1)
    {
    return 0;
    }

  if (! this->ButtonActor->GetVisibility())
    {
    return 0;
    }
  this->BuildRepresentation();

  int count = 0;
  count += this->ButtonActor->RenderTranslucentPolygonalGeometry(w);
  if (this->AnnotationVisibility)
    {
    count += this->Balloon->RenderTranslucentPolygonalGeometry(w);
    }
  if (this->SelectedLocation >= 0)
    {
    count += this->MarkerActor->RenderTranslucentPolygonalGeometry(w);
    }

  return count;
}

//-----------------------------------------------------------------------------
// Description:
// Does this prop have some translucent polygonal geometry?
int vtkVgSelectionListRepresentation::HasTranslucentPolygonalGeometry()
{
  // Nothing appears id no items defined
  if (this->GetNumberOfItems() < 1)
    {
    return 0;
    }

  if (! this->ButtonActor->GetVisibility())
    {
    return 0;
    }
  this->BuildRepresentation();

  int hasIt = 0;
  hasIt |= this->ButtonActor->HasTranslucentPolygonalGeometry();
  if (this->AnnotationVisibility)
    {
    hasIt |= this->Balloon->HasTranslucentPolygonalGeometry();
    }
  if (this->SelectedLocation >= 0)
    {
    hasIt |= this->MarkerActor->HasTranslucentPolygonalGeometry();
    }

  return hasIt;
}

//-------------------------------------------------------------------------
vtkIdType vtkVgSelectionListRepresentation::GetNumberOfItems()
{
  return static_cast<vtkIdType>(this->SelectionMap->size());
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::GetSize(double size[2])
{
  size[0] = 1.0;
  size[1] = 1.0;
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::
AddItem(vtkIdType itemID, double s, const char* str, vtkImageData* img)
{
  vtkVgSelectionMapIterator iter = this->SelectionMap->find(itemID);
  if (iter == this->SelectionMap->end())
    {
    vtkVgSelectionSetRValue ret;

    ret = this->SelectionSet->insert(vtkVgSelectionBalloon(itemID, s, str, img));
    if (ret.second)   // successful insertion
      {
      (*this->SelectionMap)[itemID] = ret.first;
      }
    this->Modified();
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::
UpdateItem(vtkIdType itemID, double s, const char* str, vtkImageData* img)
{
  vtkVgSelectionMapIterator iter = this->SelectionMap->find(itemID);
  if (iter != this->SelectionMap->end())
    {
    this->SelectionMap->erase(itemID);
    this->AddItem(itemID, s, str, img);
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::RemoveItem(vtkIdType itemID)
{
  vtkVgSelectionMapIterator iter = this->SelectionMap->find(itemID);
  if (iter != this->SelectionMap->end())
    {
    if (itemID == this->CurrentItemId)
      {
      this->CurrentItemId = -1;
      }
    vtkVgSelectionSetIterator miter = (*iter).second;
    this->SelectionSet->erase(miter);
    this->SelectionMap->erase(iter);
    this->Modified();
    }
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::
RemoveAllItems()
{
  this->SelectionMap->clear();
  this->SelectionSet->clear();
  this->CurrentItemId = -1;
  this->Modified();
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::SetListPosition(double xyz[3])
{
  this->PositionCoordinate->SetValue(xyz);
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::SetListPosition(double x, double y, double z)
{
  this->PositionCoordinate->SetValue(x, y, z);
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::GetListPosition(double xyz[3])
{
  return this->PositionCoordinate->GetValue(xyz);
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::SetListSize(double xy[2])
{
  this->Position2Coordinate->SetValue(xy[0], xy[1]);
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::SetListSize(double x, double y)
{
  this->Position2Coordinate->SetValue(x, y);
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::GetListSize(double xy[2])
{
  double p[3];
  this->Position2Coordinate->GetValue(p);
  xy[0] = p[0];
  xy[1] = p[1];
}

//-------------------------------------------------------------------------
void vtkVgSelectionListRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Orientation: ";
  if (this->Orientation == HorizontalList)
    {
    os << "Horizontal\n";
    }
  else
    {
    os << "Vertical\n";
    }

  os << indent << "LookupTable: ";
  if (this->LookupTable)
    {
    this->LookupTable->PrintSelf(os << endl, indent.GetNextIndent());
    }
  else
    {
    os << "(none)\n";
    }

  os << indent << "Balloon Offset: " << this->BalloonOffset << "\n";
}
