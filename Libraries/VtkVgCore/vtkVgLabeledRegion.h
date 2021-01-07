// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgLabeledRegion_h
#define __vtkVgLabeledRegion_h

#include "vtkProp.h"

#include <vgExport.h>

class vtkActor;
class vtkActor2D;
class vtkImageData;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper2D;
class vtkPolyDataMapper;
class vtkProperty;
class vtkProperty2D;
class vtkTextActor;
class vtkTextMapper;
class vtkTextProperty;
class vtkTexture;
class vtkTexturedActor2D;

class VTKVG_CORE_EXPORT vtkVgLabeledRegion : public vtkProp
{
public:
  enum LayoutStyle
    {
    LayoutTextToRightOfImage,
    LayoutTextBelowImage
    };

public:
  // Description:
  // Instantiate the class.
  static vtkVgLabeledRegion* New();

  // Description:
  // Standard VTK methods.
  vtkTypeMacro(vtkVgLabeledRegion, vtkProp);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify/retrieve the highlight polygon.
  virtual void SetFramePolyData(vtkPolyData* pd);
  vtkGetObjectMacro(FramePolyData, vtkPolyData);

  // Description:
  // Specify/retrieve the image to display.
  virtual void SetImage(vtkImageData* img);
  vtkGetObjectMacro(Image, vtkImageData);

  // Description:
  // Specify/retrieve the text to display.
  vtkGetStringMacro(Text);
  vtkSetStringMacro(Text);

  // Description:
  // Specify the minimum size for the image.
  vtkSetVector2Macro(ImageSize, int);
  vtkGetVector2Macro(ImageSize, int);

  // Description:
  // Get the component actors.
  vtkProp* GetFrameActor();
  vtkProp* GetImageActor();
  vtkProp* GetTextActor();

  // Description:
  // Set/get the text property (relevant only if text is shown).
  virtual void SetTextProperty(vtkTextProperty* p);
  vtkGetObjectMacro(TextProperty, vtkTextProperty);

  // Description:
  // Set/get the frame property (relevant only if text is shown).
  virtual void SetFrameProperty(vtkProperty* p);
  vtkGetObjectMacro(FrameProperty, vtkProperty);

  // Description:
  // Set/get the image property (relevant only if an image is shown).
  virtual void SetImageProperty(vtkProperty2D* p);
  vtkGetObjectMacro(ImageProperty, vtkProperty2D);

  // Description:
  // Set/Get the layout of text and image.
  vtkSetMacro(Layout, LayoutStyle);
  vtkGetMacro(Layout, LayoutStyle);

  // Description:
  // Set/get the normalized position of the label (image + text)  relative to
  // the frame bounding rectangle (0, 0 = upper left).
  vtkSetVector2Macro(NormalizedLabelPosition, double);
  vtkGetVector2Macro(NormalizedLabelPosition, double);

  // Description:
  // Set/Get the padding (in pixels) between the image and the text string.
  vtkSetClampMacro(Padding, int, 0, 100);
  vtkGetMacro(Padding, int);

  // Description:
  // Methods required by vtkProp superclass.
  virtual void ReleaseGraphicsResources(vtkWindow* w);

  virtual int HasTranslucentPolygonalGeometry();
  virtual int RenderOpaqueGeometry(vtkViewport* viewport);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport* viewport);
  virtual int RenderOverlay(vtkViewport* viewport);

protected:
  vtkVgLabeledRegion();
  ~vtkVgLabeledRegion();

  // raw text and image data
  char*         Text;
  vtkImageData* Image;

  // placement
  LayoutStyle Layout;
  int Padding;
  int ImageSize[2];
  double NormalizedLabelPosition[2];

  // text
  vtkActor2D*          TextActor;
  vtkTextMapper*       TextMapper;
  vtkTextProperty*     TextProperty;

  // image
  vtkTexture*          Texture;
  vtkPolyData*         TexturePolyData;
  vtkPoints*           TexturePoints;
  vtkPolyDataMapper2D* TextureMapper;
  vtkTexturedActor2D*  ImageActor;
  vtkProperty2D*       ImageProperty;

  // frame
  vtkActor*            FrameActor;
  vtkPolyDataMapper*   FrameMapper;
  vtkPolyData*         FramePolyData;
  vtkProperty*         FrameProperty;

  // internal variable controlling rendering process
  bool TextVisible;
  bool ImageVisible;

  vtkTimeStamp UpdateTime;

protected:
  void AdjustImageSize(double imageSize[2]);
  void Update(vtkViewport* viewport);

private:
  vtkVgLabeledRegion(const vtkVgLabeledRegion&);  // not implemented
  void operator=(const vtkVgLabeledRegion&);     // not implemented
};

#endif
