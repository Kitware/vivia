/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgJPEGReader_h
#define __vtkVgJPEGReader_h

// VTKExtensions includes.
#include "vtkVgBaseImageSource.h"

#include <vgExport.h>

// Forward declaration.
class vtkJPEGReader;

class VTKVG_CORE_EXPORT vtkVgJPEGReader : public vtkVgBaseImageSource
{
public:
  // Description:
  // Create new instance.
  static vtkVgJPEGReader* New();

  // Description:
  // Check type.
  vtkTypeMacro(vtkVgJPEGReader, vtkVgBaseImageSource);

  // Description:
  // Print values of the member variables.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return number of level of detail.
  virtual int GetNumberOfLevels() const;

  // Description:
  // Return dimensions of the image.
  virtual void GetDimensions(int dim[2]);

  // Description:
  // Check if the reader can read a given source.
  virtual bool CanRead(const std::string& source);

  // Description:
  // Return short description of reader
  virtual std::string GetShortDescription();

  // Description:
  // Return long description of reader
  virtual std::string GetLongDescription();

  // Description:
  // Create instance of reader
  static vtkVgBaseImageSource* Create();

protected:

  vtkVgJPEGReader();
  virtual ~vtkVgJPEGReader();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  vtkImageData* ImageCache;

  vtkJPEGReader* Reader;

private:
  // Not implemented.
  vtkVgJPEGReader(const vtkVgJPEGReader&);
  // Not implemented.
  void operator= (const vtkVgJPEGReader&);
};

#endif // __vtkVgJPEGReader_h
