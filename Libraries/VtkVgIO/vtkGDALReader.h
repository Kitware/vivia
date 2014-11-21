/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkGDALReader_h
#define __vtkGDALReader_h

#include <vgExport.h>

#include <vtkImageReader2.h>

// C++ includes
#include <string>
#include <vector>

class VTKVG_IO_EXPORT vtkGDALReader : public vtkImageReader2
{
public:
  static vtkGDALReader* New();
  vtkTypeMacro(vtkGDALReader, vtkImageReader2);

  vtkGDALReader();
  virtual ~vtkGDALReader();

  // Description:
  // Set input file name
  vtkSetStringMacro(FileName);
  // Get input file name
  vtkGetStringMacro(FileName);

  // Description:
  // Return proj4 spatial reference.
  const char*  GetProjectionString() const;

  // Description:
  // Return geo-referenced corner points (Upper left,
  // lower left, lower right, upper right)
  const double* GetGeoCornerPoints();

  // Description:
  // Return extent of the data
  vtkSetVector6Macro(DataExtents, int);
  int* GetDataExtent();

  // Description:
  // Set desired width and height of the image
  vtkSetVector2Macro(TargetDimensions, int);
  vtkGetVector2Macro(TargetDimensions, int);

  // Description:
  // Get raster width and heigth
  vtkGetVector2Macro(RasterDimensions, int);

  // Description:
  // Set spacing of the data in the file.
  vtkSetVector3Macro(DataSpacing, double);
  // Get spacing of the data in the file.
  vtkGetVector3Macro(DataSpacing, double);

  // Description:
  // Set origin of the data (location of first pixel in the file).
  vtkSetVector3Macro(DataOrigin, double);
  // Get origin of the data (location of first pixel in the file).
  vtkGetVector3Macro(DataOrigin, double);

  // Description:
  // Return metadata as reported by GDAL
  const std::vector<std::string>& GetMetaData();

  // Description:
  // Return domain metadata
  std::vector<std::string> GetDomainMetaData(const std::string& domain);

  // Description:
  // Return driver name which was used to read the current data
  const std::string& GetDriverShortName();
  const std::string& GetDriverLongName();

protected:
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int FillOutputPortInformation(int port,
                                        vtkInformation* info);

protected:
  char* FileName;
  double DataSpacing[3];
  double DataOrigin[3];
  int DataExtents[6];
  int TargetDimensions[2];
  int RasterDimensions[2];
  std::string Projection;
  std::string DomainMetaData;
  std::string DriverShortName;
  std::string DriverLongName;
  std::vector<std::string> Domains;
  std::vector<std::string> MetaData;

  class vtkGDALReaderInternal;
  vtkGDALReaderInternal* Implementation;

private:
  vtkGDALReader(const vtkGDALReader&); // Not implemented.
  vtkGDALReader& operator=(const vtkGDALReader&); // Not implemented.
};

#endif // __vtkGDALReader_h
