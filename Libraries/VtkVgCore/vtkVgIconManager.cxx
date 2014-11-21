/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgIconManager.h"
#include "vtkVgIcon.h"
#include "vtkObjectFactory.h"
#include "vtkPNGReader.h"
#include "vtkTexturedActor2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkTransformCoordinateSystems.h"
#include "vtkPolyData.h"
#include "vtkIconGlyphFilter.h"
#include "vtkTexture.h"
#include "vtkPointData.h"
#include "vtkVgEvent.h"
#include "vtkVgTrack.h"
#include "vtkViewport.h"
#include "vtkRenderer.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkIdTypeArray.h"
#include "vtkDoubleArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkMath.h"
#include "vtkImageData.h"
#include "vtkExtractVOI.h"

// STL includes.
#include <sstream>
#include <map>

// VTK sys includes.
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkVgIconManager);

typedef std::map<vtkIdType, vtkSmartPointer<vtkVgIcon> > IconMapType;
typedef std::map<vtkIdType, vtkSmartPointer<vtkVgIcon> >::iterator IconMapIterator;

struct EventIconInfo
{
  const char* Label;
  int Type;
};

class vtkVgIconManager::vtkInternal
{
public:
  IconMapType IconMap;
  vtkSmartPointer<vtkTexturedActor2D> Actor;
  vtkSmartPointer<vtkTexture> IconSheet;
  vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
  vtkSmartPointer<vtkIconGlyphFilter> Glypher;
  vtkSmartPointer<vtkTransformCoordinateSystems> XForm;
  vtkSmartPointer<vtkPolyData> PolyData;
  vtkSmartPointer<vtkPoints> Points;
  vtkSmartPointer<vtkIntArray> Index;
  vtkSmartPointer<vtkDoubleArray> IconScale;
  vtkSmartPointer<vtkIdTypeArray> IconIds;
  vtkSmartPointer<vtkTexture> Texture;
  vtkSmartPointer<vtkPNGReader> IconSheetReader;

  std::vector<EventIconInfo> StaticEventIcons;

  vtkInternal()
    {
    this->Actor = vtkSmartPointer<vtkTexturedActor2D>::New();
    this->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->Glypher = vtkSmartPointer<vtkIconGlyphFilter>::New();
    this->XForm = vtkSmartPointer<vtkTransformCoordinateSystems>::New();
    this->PolyData = vtkSmartPointer<vtkPolyData>::New();
    this->Points = vtkSmartPointer<vtkPoints>::New();
    this->Index = vtkSmartPointer<vtkIntArray>::New();
    this->IconScale = vtkSmartPointer<vtkDoubleArray>::New();
    this->IconIds = vtkSmartPointer<vtkIdTypeArray>::New();
    this->IconSheetReader = vtkSmartPointer<vtkPNGReader>::New();
    this->Texture = vtkSmartPointer<vtkTexture>::New();

    this->Texture->SetInputConnection(this->IconSheetReader->GetOutputPort());
    this->Texture->MapColorScalarsThroughLookupTableOff();
    this->Index->SetName("IconIndex");
    this->Index->SetNumberOfComponents(1);
    this->IconScale->SetName("IconScale");
    this->IconScale->SetNumberOfComponents(1);
    this->IconIds->SetName("IconIds");
    this->IconIds->SetNumberOfComponents(1);
    this->PolyData->SetPoints(this->Points);
    this->PolyData->GetPointData()->SetScalars(this->Index);
    this->PolyData->GetPointData()->AddArray(this->IconScale);
    this->PolyData->GetPointData()->AddArray(this->IconIds);
    this->XForm->SetInputData(this->PolyData);
    this->XForm->SetInputCoordinateSystemToWorld();
    this->XForm->SetOutputCoordinateSystemToDisplay();
    this->Glypher->SetInputConnection(this->XForm->GetOutputPort());
    this->Glypher->SetUseIconSize(true);
    this->Glypher->SetIconScalingToScalingArray();
    this->Mapper->SetInputConnection(this->Glypher->GetOutputPort());
    this->Mapper->SetScalarVisibility(false);
    this->Actor->SetMapper(this->Mapper);
    this->Actor->SetTexture(this->Texture);
    }

};


//-----------------------------------------------------------------------------
vtkVgIconManager::vtkVgIconManager()
{
  this->ImageHeight = 0;
  this->Internal = new vtkInternal();

  this->MaxIconId = 0;

  this->IconSize = 0.02;
  this->MinimumIconSize = 10;
  this->MaximumIconSize = 50;
  this->OriginalIconSize[0] = this->OriginalIconSize[1] = 0;
  this->SheetSize[0] = this->SheetSize[1] = 0;
  this->Visibility = 1; //default to visible
  this->IconOffset[0] = this->IconOffset[1] = 5;
}

//-----------------------------------------------------------------------------
vtkVgIconManager::~vtkVgIconManager()
{
  this->DeleteAllIcons();
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::SetIconSheetFileName(const char* fname)
{
  this->Internal->IconSheetReader->SetFileName(fname);
}

//-----------------------------------------------------------------------------
char* vtkVgIconManager::GetIconSheetFileName()
{
  return this->Internal->IconSheetReader->GetFileName();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkTexturedActor2D> vtkVgIconManager::GetIconActor()
{
  return this->Internal->Actor;
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::SetRenderer(vtkViewport* ren)
{
  this->Internal->XForm->SetViewport(ren);
}


//-----------------------------------------------------------------------------
vtkIdType vtkVgIconManager::
AddIcon(char* category, char* type, int visibility, int pos[2], double s)
{
  vtkIdType iconId = this->MaxIconId++;
  this->AddIcon(iconId, category, type, visibility, pos, s);

  return iconId;
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::
AddIcon(vtkIdType iconId, char* category, char* type, int visibility, int pos[2], double s)
{
  // Create an icon and add it to the map
  std::string cat = category;
  std::string lab = type;
  int iCat, iType;

  if (cat == "EVENT")
    {
    iCat = vgEventTypeDefinitions::Event;
    }
  else if (cat == "ACTIVITY")
    {
    iCat = vgEventTypeDefinitions::Activity;
    }
  else if (cat == "INTEL")
    {
    iCat = vgEventTypeDefinitions::Intel;
    }
  else
    {
    iCat = vgEventTypeDefinitions::CategoryUnknown;
    }

  if (lab == "INTEL")
    {
    iType = vgSpecialIconTypeDefinitions::Intelligence;
    }
  else if (lab == "FOLLOWER")
    {
    iType = vgSpecialIconTypeDefinitions::Follower;
    }
  else
    {
    bool found = false;
    for (size_t i = 0, end = this->Internal->StaticEventIcons.size(); i < end;
         ++i)
      {
      if (lab == this->Internal->StaticEventIcons[i].Label)
        {
        found = true;
        iType = this->Internal->StaticEventIcons[i].Type;
        break;
        }
      }
    if (!found)
      {
      iType = vgSpecialIconTypeDefinitions::Unknown;
      }
    }

  this->AddIcon(iconId, iCat, iType, visibility, pos, s);
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgIconManager::
AddIcon(int category, int type, int visibility, int pos[2], double s)
{
  vtkIdType iconId = this->MaxIconId++;
  this->AddIcon(iconId, category, type, visibility, pos, s);

  return iconId;
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::
AddIcon(vtkIdType iconId, int category, int type, int visibility, int pos[2], double s)
{
  if (iconId >= this->MaxIconId)
    {
    this->MaxIconId = iconId + 1;
    }

  vtkSmartPointer<vtkVgIcon> icon = vtkSmartPointer<vtkVgIcon>::New();
  icon->SetCategory(category);
  icon->SetType(type);
  icon->SetVisibility(visibility);
  icon->SetPosition(pos[0], pos[1]);
  icon->SetScale(s);
  this->Internal->IconMap[iconId] = icon;
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::DeleteIcon(vtkIdType iconId)
{
  IconMapIterator end = this->Internal->IconMap.end();
  IconMapIterator iter = this->Internal->IconMap.find(iconId);
  if (iter != end)
    {
    this->Internal->IconMap.erase(iter);
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::DeleteAllIcons()
{
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();
  this->Internal->IconMap.erase(begin, end);
}


//-----------------------------------------------------------------------------
int vtkVgIconManager::GetNumberOfIcons()
{
  return static_cast<int>(this->Internal->IconMap.size());
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::AllIconsOn()
{
  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  for (iter = begin; iter != end; ++iter)
    {
    iter->second->SetVisibility(1);
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::AllIconsOff()
{
  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  for (iter = begin; iter != end; ++iter)
    {
    iter->second->SetVisibility(0);
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::IntelligenceIconsOn()
{
  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  for (iter = begin; iter != end; ++iter)
    {
    if (iter->second->GetCategory() == vgEventTypeDefinitions::Intel)
      {
      iter->second->SetVisibility(1);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::IntelligenceIconsOff()
{
  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  for (iter = begin; iter != end; ++iter)
    {
    if (iter->second->GetCategory() == vgEventTypeDefinitions::Intel)
      {
      iter->second->SetVisibility(0);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::EventIconsOn()
{
  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  for (iter = begin; iter != end; ++iter)
    {
    if (iter->second->GetCategory() == vgEventTypeDefinitions::Event)
      {
      iter->second->SetVisibility(1);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::EventIconsOff()
{
  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  for (iter = begin; iter != end; ++iter)
    {
    if (iter->second->GetCategory() == vgEventTypeDefinitions::Event)
      {
      iter->second->SetVisibility(0);
      }
    }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgIcon> vtkVgIconManager::
GetIcon(vtkIdType iconId)
{
  IconMapIterator end = this->Internal->IconMap.end();
  IconMapIterator iter = this->Internal->IconMap.find(iconId);

  return iter == end ? 0 : iter->second;
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::
SetIconVisibility(int category, int type, int visibility)
{
  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  for (iter = begin; iter != end; ++iter)
    {
    if (iter->second->GetCategory() == category)
      {
      if (iter->second->GetType() == type)
        {
        iter->second->SetVisibility(visibility);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::
SetIconVisibility(vtkIdType iconId, int visibility)
{
  IconMapIterator iter = this->Internal->IconMap.find(iconId);
  IconMapIterator end = this->Internal->IconMap.end();
  if (iter != end)
    {
    iter->second->SetVisibility(visibility);
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::
SetIconScale(vtkIdType iconId, double scale)
{
  IconMapIterator iter = this->Internal->IconMap.find(iconId);
  IconMapIterator end = this->Internal->IconMap.end();
  if (iter != end)
    {
    iter->second->SetScale(scale);
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::
UpdateIconPosition(vtkIdType iconId, int pos[2])
{
  IconMapIterator iter = this->Internal->IconMap.find(iconId);
  IconMapIterator end = this->Internal->IconMap.end();
  if (iter != end)
    {
    iter->second->SetPosition(pos);
    }
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::
GetIconPosition(vtkIdType iconId, int pos[2])
{
  IconMapIterator iter = this->Internal->IconMap.find(iconId);
  IconMapIterator end = this->Internal->IconMap.end();
  if (iter != end)
    {
    iter->second->GetPosition(pos);
    }
}


//-----------------------------------------------------------------------------
int vtkVgIconManager::LoadIcons(const char* filename)
{
  this->DeleteAllIcons();

  return this->AddIcons(filename);
}

//-----------------------------------------------------------------------------
int vtkVgIconManager::AddIcons(const char* filename)
{
  ifstream in(filename);
  if (!in.good())
    {
    vtkErrorMacro("can't open events file!");
    return VTK_ERROR;
    }
  char fullLine[1024], fname[2048], category[1024], label[1024];
  int iconId; // not vtkIdType because we are reading it via sscanf
  int size, x, y;

  // Read the comment line
  in.getline(fullLine, 1024, '\n');

  // Read the parameters for the icons
  in.getline(fullLine, 1024, '\n');
#ifdef _WIN32
  sscanf_s(fullLine, "%d %d %d %s", &size, &x, &y,
           fname, static_cast<unsigned>(sizeof(fname)));
#else
  sscanf(fullLine, "%d %d %d %s", &size, &x, &y, fname);
#endif

  std::string fullPath("");

  if (vtksys::SystemTools::FileIsFullPath(fname))
    {
    fullPath = fname;
    }
  else
    {
    std::string path =
      vtksys::SystemTools::GetFilenamePath(std::string(filename));
    path.append("/");

    fullPath = path + std::string(fname);
    }

  if (!vtksys::SystemTools::FileExists(fullPath.c_str(), true))
    {
    vtkErrorMacro("Cannot find icon sheet file ( " << fullPath << " )\n");
    return VTK_ERROR;
    }

  this->SetIconSheetFileName(fullPath.c_str());
  this->Internal->Glypher->SetIconSize(size, size);
  this->SetOriginalIconSize(size, size);
  this->SetSheetSize(x, y);

  // Read the comment line
  in.getline(fullLine, 1024, '\n');

  // Read the icon definitions
  int pos[2];
  while (!in.eof())
    {
    in.getline(fullLine, 1024, '\n');
#ifdef _WIN32
    sscanf_s(fullLine, "%d %s %s %d %d", &iconId,
             category, static_cast<unsigned>(sizeof(category)),
             label, static_cast<unsigned>(sizeof(label)),
             pos, pos + 1);
#else
    sscanf(fullLine, "%d %s %s %d %d", &iconId, category, label, pos, pos + 1);
#endif
    this->AddIcon(iconId, category, label, 1, pos, 1.0);
    } //while

  return VTK_OK;
}

//-----------------------------------------------------------------------------
// There are two types of icons to update: 1) static icons defined in the
// icon file, and 2) track icons which are related to the tracks and current events.
void vtkVgIconManager::UpdateIconActors(vtkIdType vtkNotUsed(frameIndex))
{
  // Check to make sure there is something to see
  if (!this->Visibility)
    {
    this->Internal->Actor->VisibilityOff();
    return;
    }
  this->Internal->Actor->VisibilityOn();

  IconMapIterator iter;
  IconMapIterator begin = this->Internal->IconMap.begin();
  IconMapIterator end = this->Internal->IconMap.end();

  // Compute the current icon size. The coordinate system transform holds the renderer.
  int iconSize = 16; //fallback value
  vtkViewport* ren = this->Internal->XForm->GetViewport();
  if (ren)
    {
    int* sze = ren->GetSize();
    if (sze[0] > 0 && sze[1] > 0)
      {
      int xSze = static_cast<int>(this->IconSize * sze[0]);
      int ySze = static_cast<int>(this->IconSize * sze[1]);
      iconSize = (xSze < ySze ? xSze : ySze);
      iconSize = (iconSize < this->MinimumIconSize ? this->MinimumIconSize : iconSize);
      iconSize = (iconSize > this->MaximumIconSize ? this->MaximumIconSize : iconSize);
      }
    }

  // First update the texture information
  this->Internal->Glypher->SetIconSize(iconSize, iconSize);
  this->Internal->Glypher->
  SetIconSheetSize(this->SheetSize[0]*iconSize, this->SheetSize[1]*iconSize);
  this->Internal->Glypher->SetOffset(this->IconOffset);

  // Reset the pipeline information to empty state
  this->Internal->Points->Reset();
  this->Internal->Index->Reset();
  this->Internal->IconScale->Reset();
  this->Internal->IconIds->Reset();

  // Loop over all icons, create a point for the visibile ones. The glyph
  // process also requires an integer id to map into the sheet icons.
  int pos[2], type;
  vtkIdType ptId;
  for (iter = begin; iter != end; ++iter)
    {
    if (iter->second->GetVisibility())
      {
      type = iter->second->GetType();
      iter->second->GetPosition(pos);

      if (type >= 0 && type != vgSpecialIconTypeDefinitions::Unknown)
        {
        ptId = this->Internal->Points->InsertNextPoint(pos[0], pos[1], 0.1);
        this->Internal->IconScale->InsertTuple1(ptId, iter->second->GetScale());
        this->Internal->IconIds->InsertTuple1(ptId, iter->first);
        this->Internal->Index->InsertTuple1(ptId, type);
        }
      }
    }

  this->Internal->Points->Modified();
}

//-----------------------------------------------------------------------------
vtkIdType vtkVgIconManager::Pick(double renX, double renY, vtkRenderer*)
{
  // Check to make sure that a renderer has been defined, and that output
  // is available from the glyph filter.
  if (! this->Internal->XForm->GetViewport() ||
      ! this->Internal->Glypher->GetOutput() ||
      this->Internal->Glypher->GetOutput()->GetNumberOfPoints() < 1)
    {
    return -1;
    }

  // Okay we have some data to pick against. The glypher creates quads for each
  // point, and the point coordinates are in display coordinates. For now we will
  // simply look to see if the pick occurs in a quad.
  vtkPolyData* pd = this->Internal->Glypher->GetOutput();
  vtkCellArray* quads = pd->GetPolys();
  vtkPoints* points = pd->GetPoints();
  vtkIdTypeArray* iconIds = dynamic_cast<vtkIdTypeArray*>(pd->GetCellData()->GetArray("IconIds"));

  vtkIdType cellId, npts, *pts, pickedCellId = (-1);
  double x0[3], x2[3];
  for (quads->InitTraversal(), cellId = 0; quads->GetNextCell(npts, pts); ++cellId)
    {
    // get the two opposing points of the quad
    points->GetPoint(pts[0], x0);
    points->GetPoint(pts[2], x2);

    // determine if the quad contains the pick. Note that these quads may overlay one
    // another, and the quad rendered last will win. Hence we return the last quad that
    // contains the pick point.
    if (x0[0] <= renX && renX <= x2[0] &&
        x0[1] <= renY && renY <= x2[1])
      {
      pickedCellId = cellId;
      }
    }

  // Return the icon id or -1 if nothing picked
  if (pickedCellId < 0)
    {
    return -1; //nothing picked
    }
  else
    {
    return iconIds->GetValue(pickedCellId);
    }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vtkVgIconManager::GetIconImage(int type)
{
  // Make sure information is valid
  if (type < 0 || type > vgSpecialIconTypeDefinitions::Unknown)
    {
    return NULL;
    }

  // Compute the location of the icon in the icon sheet.
  // Note that the y-coordinate has to be flipped.
  int sX = type % (this->SheetSize[0]);
  int sY = type / (this->SheetSize[0]);
  sY = this->SheetSize[1] - sY - 1;

  int sizeX = this->OriginalIconSize[0];
  int sizeY = this->OriginalIconSize[1];

  // Now specify the offset information into the icon sheet, and
  // extract the icon image.
  vtkSmartPointer<vtkExtractVOI> voi = vtkSmartPointer<vtkExtractVOI>::New();
  voi->SetInputConnection(this->Internal->IconSheetReader->GetOutputPort());
  voi->SetVOI(sX * sizeX, sizeX * (sX + 1) - 1,
              sY * sizeY, sizeY * (sY + 1) - 1, 0, 0);
  voi->Update();

  // Return the image
  vtkSmartPointer<vtkImageData> iconImage = voi->GetOutput();
  return iconImage;
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::RegisterStaticEventIcon(const char* label, int type)
{
  EventIconInfo eii;
  eii.Label = label;
  eii.Type = type;
  this->Internal->StaticEventIcons.push_back(eii);
}

//-----------------------------------------------------------------------------
void vtkVgIconManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
