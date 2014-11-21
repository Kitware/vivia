/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgBaseImageSource_h
#define __vtkVgBaseImageSource_h

#include "vtkVgTimeStamp.h"

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkSmartPointer.h>

// Forward declarations
class vtkMatrix4x4;

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgBaseImageSource : public vtkImageAlgorithm
{
public:
  // Description:
  // Check type.
  vtkTypeMacro(vtkVgBaseImageSource, vtkImageAlgorithm);

  // Description:
  // Print values of the member variables.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Set the flag to true to allow the use of
  // output resolution in \c SetOutputResolution
  // and vice versa.
  vtkSetMacro(UseOutputResolution, bool);
  vtkGetMacro(UseOutputResolution, bool);
  vtkBooleanMacro(UseOutputResolution, bool);

  // Description:
  // Request a particular resolution. This is useful
  // if the actual image is much bigger than the
  // dimension of the display viewport or window.
  void GetOutputResolution(int& w, int& h) const;
  int  SetOutputResolution(int w, int h);

  // Description:
  // The extent of pixels to read.
  vtkSetVector4Macro(ReadExtents, int);
  vtkGetVector4Macro(ReadExtents, int);

  // Description:
  // If the image has multiple level of details
  // and if the reader supports it set the level of
  // detail to one particular leve.
  // @NOTE: A lower number means higher resolution.
  vtkGetMacro(Level, int);
  vtkSetMacro(Level, int);

  // Description:
  // Given the ratio of width of the camera viewport
  // to window viewport calculate and use the
  // correct level of detail.
  vtkGetMacro(Scale, double);
  vtkSetMacro(Scale, double);

  // Description:
  // Set the file to read. Required full path to the
  // file.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  // Description:
  // Set the origin that will be set in the pipeline.
  vtkSetVector3Macro(Origin, double);
  vtkGetVector3Macro(Origin, double);

  // Description:
  // Set the spacing that will be set in the pipeline.
  vtkSetVector3Macro(Spacing, double);
  vtkGetVector3Macro(Spacing, double);

  // Description:
  // Return the dimensions of the image data.
  virtual void GetDimensions(int dim[2]) = 0;

  // Description:
  // Get the dimensions of the entire image and return true if
  // supported by the reader).
  virtual bool GetRasterDimensions(int vtkNotUsed(dim)[2])
    {
    return false;
    }

  // Description:
  // Return the number of level of details exists
  // or supported by this reader.
  virtual int GetNumberOfLevels() const;

  // Description:
  // Get image timestamp
  virtual vtkVgTimeStamp GetImageTimeStamp()
    {
    return this->ImageTimeStamp;
    }

  // Description:
  // Check if a given source (file, extension) can be read by
  // this source.
  virtual bool CanRead(const std::string& source) = 0;

  // Description:
  // Return a short description of the source (for display purposes).
  virtual std::string GetShortDescription() = 0;

  // Description:
  // Return a long description of the source.
  virtual std::string GetLongDescription() = 0;

  // Description:
  // FIXME
  virtual vtkSmartPointer<vtkMatrix4x4> GetImageToWorldMatrix();

  // Description:
  // FIXME
  virtual const double* GetGeoCornerPoints()
    {
    return 0;
    }

protected:

  vtkVgBaseImageSource();
  virtual ~vtkVgBaseImageSource();

// Description:
// Derived classes need to implement this function.
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

// Description:
// Derived classes need to implement this function.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:

  vtkVgBaseImageSource(const vtkVgBaseImageSource&);  // Not implemented.
  void operator=(const vtkVgBaseImageSource&);        // Not implemented.


// Data members.
protected:

  bool    UseOutputResolution;
  int     OutputResolution[2];
  int     ReadExtents[4];

  int     Level;
  double  Scale;

  double  Origin[3];
  double  Spacing[3];

  char*   FileName;

  vtkVgTimeStamp ImageTimeStamp;
};

#endif // __vtkVgBaseImageSource_h
