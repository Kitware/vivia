// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
  // Set desired width and height of the image
  vtkSetVector2Macro(TargetDimensions, int);
  vtkGetVector2Macro(TargetDimensions, int);

  // Description:
  // Get raster width and heigth
  vtkGetVector2Macro(RasterDimensions, int);

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
