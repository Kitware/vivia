// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTIFReader_h
#define __vtkVgTIFReader_h

// VTKExtensions includes.
#include "vtkVgBaseImageSource.h"

#include <vgExport.h>

// Forward declaration.
class vtkTIFFReader;

class VTKVG_CORE_EXPORT vtkVgTIFReader : public vtkVgBaseImageSource
{
public:
  // Description:
  // Create new instance.
  static vtkVgTIFReader* New();

  // Description:
  // Check type.
  vtkTypeMacro(vtkVgTIFReader, vtkVgBaseImageSource);

  // Description:
  // Print values of the member variables.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return number of level of detail.
  virtual int GetNumberOfLevels() const;

  // Description:
  // Return dimensions of the image.
  virtual void GetDimensions(int dim[2]);

  virtual bool CanRead(const std::string& source) const;

  virtual std::string GetShortDescription() const;

  virtual std::string GetLongDescription() const;

  static vtkVgBaseImageSource* Create();

protected:

  vtkVgTIFReader();
  virtual ~vtkVgTIFReader();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  vtkImageData* ImageCache;

  vtkTIFFReader* Reader;

private:

  vtkVgTIFReader(const vtkVgTIFReader&);        // Not implemented.
  void operator= (const vtkVgTIFReader&);       // Not implemented.
};

#endif // __vtkVgTIFReader_h
