/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgGDALReader_h
#define __vtkVgGDALReader_h

#include <vgExport.h>

#include <vtkVgBaseImageSource.h>

#include <istream>

// Forward declaration.
class vtkGDALReader;
class vtkMatrix4x4;

class VTKVG_IO_EXPORT vtkVgGDALReader : public vtkVgBaseImageSource
{
public:
  // Description:
  // Create new instance.
  static vtkVgGDALReader* New();

  // Description:
  // Check type.
  vtkTypeMacro(vtkVgGDALReader, vtkVgBaseImageSource);

  // Description:
  // Print values of the member variables.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Request a particular resolution. This is useful
  // if the actual image is much bigger than the
  // dimension of the display viewport or window.
  virtual int SetOutputResolution(int w, int h);

  // Description:
  // Return number of level of detail.
  virtual int GetNumberOfLevels() const;

  // Description:
  // Return geo-referenced corner points (Upper left,
  // lower left, lower right, upper right)
  virtual const double* GetGeoCornerPoints();

  // Description:
  // Get the dimension of the entire image; returns whether or not it is
  // supported by this reader.
  virtual bool GetRasterDimensions(int dim[2]);

  // Description:
  // Return dimensions of the image.
  virtual void GetDimensions(int dim[2]);

  // Description:
  // Get image timestamp
  virtual vtkVgTimeStamp GetImageTimeStamp();

  virtual bool CanRead(const std::string& source) const;

  virtual std::string GetShortDescription() const;
  virtual std::string GetLongDescription() const;

  virtual vtkSmartPointer<vtkMatrix4x4> GetImageToWorldMatrix();

  static vtkVgBaseImageSource* Create();

protected:

  vtkVgGDALReader();
  virtual ~vtkVgGDALReader();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  void ComputeImageTimeStamp();

  vtkImageData* ImageCache;

  vtkGDALReader* Reader;

  vtkVgTimeStamp ImageTimeStamp;

private:

  vtkVgGDALReader(const vtkVgGDALReader&);  // Not implemented.
  void operator= (const vtkVgGDALReader&);  // Not implemented.
};

#endif // __vtkVgGDALReader_h
