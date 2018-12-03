/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgKWAReader_h
#define __vtkVgKWAReader_h

#include "vtkVgBaseImageSource.h"

#include <vgExport.h>

#include <memory>

// Forward declaration.
class vtkKWAReader;

class VTKVG_CORE_EXPORT vtkVgKWAReader : public vtkVgBaseImageSource
{
public:
  // Description:
  // Create new instance.
  static vtkVgKWAReader* New();

  // Description:
  // Check type.
  vtkTypeMacro(vtkVgKWAReader, vtkVgBaseImageSource);

  // Description:
  // Print values of the member variables.
  virtual void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Return number of level of detail.
  virtual int GetNumberOfLevels() const override;

  // Description:
  // Return dimensions of the image.
  virtual void GetDimensions(int dim[2]) override;

  // Description:
  // Get image timestamp
  virtual vtkVgTimeStamp GetImageTimeStamp() override;

  virtual bool CanRead(const std::string& source) const override;

  virtual std::string GetShortDescription() const override;

  virtual std::string GetLongDescription() const override;

  static vtkVgBaseImageSource* Create();

protected:
  class vtkInternal;
  std::unique_ptr<vtkInternal> Internal;

  vtkVgKWAReader();
  virtual ~vtkVgKWAReader();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector) override;

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

private:
  vtkVgKWAReader(const vtkVgKWAReader&) = delete;
  void operator= (const vtkVgKWAReader&) = delete;
};

#endif
