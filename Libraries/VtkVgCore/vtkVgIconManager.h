/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgIconManager - manage icons in the display
// .SECTION Description
// This manages a set of icons in the display. Various options are available
// for turning on and off different types of icons, and deleting them.
//
// Currently this class is aware of two types of icons: intelligence icons and
// event icons. Methods for turning these on and off are available.

#ifndef __vtkVgIconManager_h
#define __vtkVgIconManager_h

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkVgIcon.h"
#include "vtkVgTypeDefs.h"

#include <vgExport.h>

class vtkTexturedActor2D;
class vtkViewport;
class vtkRenderer;
class vtkImageData;


class VTKVG_CORE_EXPORT vtkVgIconManager : public vtkObject
{
public:
  // Description:
  // Standard VTK functions.
  static vtkVgIconManager* New();
  vtkTypeMacro(vtkVgIconManager, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the Height of the image, which is necessary to convert
  // coordinates from upper left to lower left.
  vtkSetMacro(ImageHeight, double);
  vtkGetMacro(ImageHeight, double);

  // Description:
  // Method to add/remove an icon.
  void AddIcon(vtkIdType iconId, int category, int type, int visibility, int pos[2], double scale);
  vtkIdType AddIcon(int category, int type, int visibility, int pos[2], double scale);
  void AddIcon(vtkIdType iconId, char* category, char* type, int visibility, int pos[2], double scale);
  vtkIdType AddIcon(char* category, char* type, int visibility, int pos[2], double scale);
  vtkSmartPointer<vtkVgIcon> GetIcon(vtkIdType iconId);
  void DeleteIcon(vtkIdType iconId);
  void DeleteAllIcons();

  // Description:
  // Method for controlling visibility of all icons. This method does not affect
  // the state of any particular icon, it affects all of them.
  vtkSetMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkBooleanMacro(Visibility, int);

  // Description:
  // Methods for turning on/off different types of icons. The icon type
  // is contained by the vtkVgIcon. These methods key in on this type.
  int GetNumberOfIcons();
  void AllIconsOn();
  void AllIconsOff();
  void IntelligenceIconsOn();
  void IntelligenceIconsOff();
  void EventIconsOn();
  void EventIconsOff();
  void SetIconVisibility(int category, int type, int visibility);
  void SetIconVisibility(vtkIdType iconId, int visibility);
  void SetIconScale(vtkIdType iconId, double scale);

  // Description:
  // Specify file name for the icon sheet.
  virtual void SetIconSheetFileName(const char*);
  char* GetIconSheetFileName();

  // Description:
  // Specify the icon size. This is expressed as a fraction of the shortest
  // dimension of the rendering window. This normalized coordinate system
  // enables the icon size to change as the window is resized.
  vtkSetClampMacro(IconSize, double, 0.001, 0.25);
  vtkGetMacro(IconSize, double);

  // Description:
  // Specify the icon offset, for example to offset from the track head. This
  // is expressed in pixels (display coordinates).
  vtkSetVector2Macro(IconOffset, int);
  vtkGetVector2Macro(IconOffset, int);

  // Description:
  // Specify the maximum size of the icon (in pixels). The IconSize is
  // initially used to compute the icon size in pixels, MinimumIconSize
  // is used to make sure that the icon is at least this big.
  vtkSetClampMacro(MinimumIconSize, int, 1, 1000);
  vtkGetMacro(MinimumIconSize, int);

  // Description:
  // Specify the maximum size of the icon (in pixels). The IconSize is
  // initially used to compute the icon size in pixels, MaximumIconSize
  // is used to make sure that the icon is no bigger than this.
  vtkSetClampMacro(MaximumIconSize, int, 10, 2000);
  vtkGetMacro(MaximumIconSize, int);

  // Description:
  // Specify the original icon dimensions.
  vtkSetVector2Macro(OriginalIconSize, int);
  vtkGetVector2Macro(OriginalIconSize, int);

  // Description:
  // Specify the icon sheet size (number of icons in the x-direction by
  // number of icons in the y-direction).
  vtkSetVector2Macro(SheetSize, int);
  vtkGetVector2Macro(SheetSize, int);

  // Description:
  // Retrieve the actor used to display the icons.
  vtkSmartPointer<vtkTexturedActor2D> GetIconActor();

  // Description:
  // Load events from a file. The AddIcons() method adds to what is already
  // there. Note that the file has an id, using the same id more than once
  // will overwrite the previous icon.
  int LoadIcons(const char* filename);
  int AddIcons(const char* filename);

  // Description:
  // Called every frame, update the icons for display.
  void UpdateIconActors(vtkIdType frameIndex);

  // Description:
  // The icons require a renderer for coordinate transformation. This must be
  // called before UpdateIconActors() is invoked.
  void SetRenderer(vtkViewport* ren);

  // Description:
  // Update the position of an existing icon.
  void UpdateIconPosition(vtkIdType iconId, int pos[2]);
  void GetIconPosition(vtkIdType iconId, int pos[2]);


  // Description:
  // Pick operation from display (i.e., pixel) coordinates in the current
  // renderer. Return the picked iconId if a successful pick occurs,
  // otherwise return -1. Note that the pick operation takes into account the
  // visibility flags of the icons.
  vtkIdType Pick(double renX, double renY, vtkRenderer* ren);

  // Description:
  // Convenience method to return the image that defines a particular type of icon.
  vtkSmartPointer<vtkImageData> GetIconImage(int type);

  // Description:
  // Used when loading static events from a file.
  void RegisterStaticEventIcon(const char* label, int type);

protected:
  // Description:
  // Constructor / Destructor.
  vtkVgIconManager();
  ~vtkVgIconManager();

private:
  vtkVgIconManager(const vtkVgIconManager&); // Not implemented.
  void operator=(const vtkVgIconManager&);  // Not implemented.

  int Visibility; //control visibility of all icons

  double ImageHeight;
  double IconSize; //fraction of rendering window
  int    MinimumIconSize; //in pixels
  int    MaximumIconSize; //in pixels
  int    OriginalIconSize[2]; //in pixels
  int    SheetSize[2];    //in pixels
  int    IconOffset[2];   //in pixels

  class vtkInternal;
  vtkInternal* Internal;

  vtkIdType MaxIconId;
};

#endif // __vtkVgIconManager_h
