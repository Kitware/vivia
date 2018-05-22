/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgSGIReader_h
#define __vtkVgSGIReader_h

// VTKExtensions includes.
#include "vtkVgBaseImageSource.h"

#include <vgExport.h>

// Forward declaration.
class vtkSGIReader;

class VTKVG_CORE_EXPORT vtkVgSGIReader : public vtkVgBaseImageSource
{
public:
  // Description:
  // Create new instance.
  static vtkVgSGIReader* New();

  // Description:
  // Check type.
  vtkTypeMacro(vtkVgSGIReader, vtkVgBaseImageSource);

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
  class vtkInternal;
  vtkInternal* Internal;

  vtkVgSGIReader();
  virtual ~vtkVgSGIReader();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkVgSGIReader(const vtkVgSGIReader&);        // Not implemented.
  void operator= (const vtkVgSGIReader&);       // Not implemented.
};

#endif
