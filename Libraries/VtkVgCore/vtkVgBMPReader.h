/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgBMPReader_h
#define __vtkVgBMPReader_h

// VTKExtensions includes.
#include "vtkVgBaseImageSource.h"

#include <vgExport.h>

// Forward declaration.
class vtkBMPReader;

class VTKVG_CORE_EXPORT vtkVgBMPReader : public vtkVgBaseImageSource
{
public:
  // Description:
  // Create new instance.
  static vtkVgBMPReader* New();

  // Description:
  // Check type.
  vtkTypeMacro(vtkVgBMPReader, vtkVgBaseImageSource);

  // Description:
  // Print values of the member variables.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return number of level of detail.
  virtual int GetNumberOfLevels() const;

  // Description:
  // Return dimensions of the image.
  virtual void GetDimensions(int dim[2]);

  virtual bool CanRead(const std::string& source);

  virtual std::string GetShortDescription();

  virtual std::string GetLongDescription();

  static vtkVgBaseImageSource* Create();

protected:

  vtkVgBMPReader();
  virtual ~vtkVgBMPReader();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  vtkImageData* ImageCache;

  vtkBMPReader* Reader;

private:

  vtkVgBMPReader(const vtkVgBMPReader&);        // Not implemented.
  void operator= (const vtkVgBMPReader&);       // Not implemented.
};

#endif // __vtkVgBMPReader_h
