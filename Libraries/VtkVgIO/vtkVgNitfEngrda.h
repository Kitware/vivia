/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgNitfEngrda_h
#define __vtkVgNitfEngrda_h

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkTypeTraits.h>

// C++ includes
#include <map>
#include <string>

class vtkVgNitfEngrda;

// Different types for the ENGRDA data
enum vtkVgNitfEngrdaType
{
  VG_NITF_ENGRDA_TYPE_ASCII,
  VG_NITF_ENGRDA_TYPE_BINARY,
  VG_NITF_ENGRDA_TYPE_UINT_8,
  VG_NITF_ENGRDA_TYPE_INT_8,
  VG_NITF_ENGRDA_TYPE_UINT_16,
  VG_NITF_ENGRDA_TYPE_INT_16,
  VG_NITF_ENGRDA_TYPE_UINT_32,
  VG_NITF_ENGRDA_TYPE_INT_32,
  VG_NITF_ENGRDA_TYPE_UINT_64,
  VG_NITF_ENGRDA_TYPE_INT_64,
  VG_NITF_ENGRDA_TYPE_FLOAT,
  VG_NITF_ENGRDA_TYPE_DOUBLE,
  VG_NITF_ENGRDA_TYPE_COMPLEX_FLOAT,
  VG_NITF_ENGRDA_TYPE_COMPLEX_DOUBLE,
  VG_NITF_ENGRDA_TYPE_UNKNOWN
};

// Container for a single data element in a set of ENGRD records
class vtkVgNitfEngrdaElement: public vtkObject
{
public:
  vtkVgNitfEngrdaElement(void);

  static vtkVgNitfEngrdaElement* New();

  const std::string& GetLabel(void);
  vtkVgNitfEngrdaType GetType(void);

  // Description:
  // Get the raw data pointer
  void* GetData(void);

  // Description:
  // Get the data formatted as a string.
  // Return false if type is not ASCII
  bool GetData(std::string& value);

  // Description:
  // Get the units of the stored data
  // Return false if type is not ASCII
  std::string& GetUnits(void);

protected:
  ~vtkVgNitfEngrdaElement(void);

private:
  friend class vtkVgNitfEngrda;

  std::string Label;
  size_t MatrixColumnCount;
  size_t MatrixRowCount;
  char Type;
  size_t DataTypeSize;
  std::string DataUnits;
  size_t DataCount;

  // FIXME: Extend for other data types
  union
    {
    char* Data;
    int DataInt;
    float DataFloat;
    double DataDouble;
    };
};
typedef vtkSmartPointer<vtkVgNitfEngrdaElement> vtkVgNitfEngrdaElementRefPtr;


// Parser for NITF ENGRDA metadata blocks.
// This is implemented based on the defined standards in:
// "Compendium of Controlled Extensions (CE) for the National Imagery
// Transmission Format (NITF) Volume 1, Tagged Record Extensions, Version 4.0"
// Specifically Appendix N:
// "Engineering Data (ENGRD) Support Data Extension (SDE)"
//
// At the time of writing, January 2012, this document is available at:
// https://nsgreg.nga.mil/NSGDOC/documents/STDI-0002-1_4.0%20CE%20NITF.zip
class vtkVgNitfEngrda
{
public:
  typedef std::map<std::string, vtkVgNitfEngrdaElementRefPtr>::iterator
    iterator;
  typedef std::map<std::string, vtkVgNitfEngrdaElementRefPtr>::const_iterator
    const_iterator;

  vtkVgNitfEngrda();

  // Description:
  // Parse engrda metadata from a given data buffer
  bool Parse(const char* data, size_t len);

  // Description:
  // Parse engrda metadata from a given data buffer
  bool Parse(const std::string& data)
    {
    return this->Parse(data.c_str(), data.size());
    }

  // Description:
  // Rietrieve the Unique Source System Name
  const std::string& GetSource(void) const;

  // Description:
  // Retrieve a data element given it's label
  const vtkVgNitfEngrdaElementRefPtr Get(const std::string& label) const;

  // Description:
  // Retrieve an iterator to the start of all data elements
  vtkVgNitfEngrda::const_iterator Begin(void) const;

  // Description:
  // Retrieve an iterator to the end of all data elements
  vtkVgNitfEngrda::const_iterator End(void) const;

private:
  // Unique Source System Name
  std::string Resource;

  // All records
  std::map<std::string, vtkVgNitfEngrdaElementRefPtr> Elements;
};

#endif
