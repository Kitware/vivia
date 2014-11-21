/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgSpaceConversion_h
#define __vtkVgSpaceConversion_h

#include <vgExport.h>

// Forward declarations
class vtkRenderer;

class VTKVG_CORE_EXPORT vtkVgSpaceConversion
{
public:
  static void WorldToView(vtkRenderer* renderer, double in[4], double out[3]);
  static void WorldToDisplay(vtkRenderer* renderer, double in[4], double out[3]);
  static void ViewToDisplay(vtkRenderer* renderer, double in[3], double out[3]);
  static void ViewToWorld(vtkRenderer* renderer, double in[3], double out[4]);
  static void DisplayToView(vtkRenderer* renderer, double in[3], double out[3]);
  static void DisplayToWorld(vtkRenderer* renderer, double in[3], double out[4]);

  static void ViewToWorldNormalized(vtkRenderer* renderer, double in[3], double out[3]);
  static void DisplayToWorldNormalized(vtkRenderer* renderer, double in[3], double out[3]);
};


#endif // __vtkVgSpaceConversion_h
