/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpBoundingRegion_h
#define __vpBoundingRegion_h

class vtkAbstractWidget;
class vtkPolyData;

// Abstract base class for bounding region editors
class vpBoundingRegion
{
public:
  virtual ~vpBoundingRegion() {};

  virtual void Initialize(vtkPolyData* pd) = 0;

  virtual vtkPolyData* GetPolyData() = 0;
  virtual vtkAbstractWidget* GetWidget() = 0;

  virtual void SetVisible(int visible) = 0;
  virtual int  GetVisible() = 0;

  virtual void SetLineColor(double r, double g, double b) = 0;

  virtual bool CanInteract(int X, int Y) = 0;
};

#endif // __vpBoundingRegion_h
