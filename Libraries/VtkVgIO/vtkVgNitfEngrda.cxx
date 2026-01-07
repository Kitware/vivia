// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgNitfEngrda.h"

// VTK includes
#include <vtkObjectFactory.h>

// C++ includes
#include <cstring>
#include <sstream>
#include <iostream>

vtkStandardNewMacro(vtkVgNitfEngrdaElement);

//----------------------------------------------------------------------------
vtkVgNitfEngrdaElement::vtkVgNitfEngrdaElement(void) : Data(NULL)
{
}

//----------------------------------------------------------------------------
vtkVgNitfEngrdaElement::~vtkVgNitfEngrdaElement(void)
{
  if (this->Data)
    {
    delete[] this->Data;
    this->Data = NULL;
    }
}

//----------------------------------------------------------------------------
const std::string& vtkVgNitfEngrdaElement::GetLabel(void)
{
  return this->Label;
}

//----------------------------------------------------------------------------
vtkVgNitfEngrdaType vtkVgNitfEngrdaElement::GetType(void)
{
  switch (this->Type)
    {
    case 'A': return VG_NITF_ENGRDA_TYPE_ASCII;
    case 'B': return VG_NITF_ENGRDA_TYPE_BINARY;
    case 'I':
      switch (this->DataTypeSize)
        {
        case 1: return VG_NITF_ENGRDA_TYPE_UINT_8;
        case 2: return VG_NITF_ENGRDA_TYPE_UINT_16;
        case 4: return VG_NITF_ENGRDA_TYPE_UINT_32;
        case 8: return VG_NITF_ENGRDA_TYPE_UINT_64;
        default: return VG_NITF_ENGRDA_TYPE_UNKNOWN;
        }
    case 'S':
      switch (this->DataTypeSize)
        {
        case 1: return VG_NITF_ENGRDA_TYPE_INT_8;
        case 2: return VG_NITF_ENGRDA_TYPE_INT_16;
        case 4: return VG_NITF_ENGRDA_TYPE_INT_32;
        case 8: return VG_NITF_ENGRDA_TYPE_INT_64;
        default: return VG_NITF_ENGRDA_TYPE_UNKNOWN;
        }
    case 'R':
      switch (this->DataTypeSize)
        {
        case 4: return VG_NITF_ENGRDA_TYPE_FLOAT;
        case 8: return VG_NITF_ENGRDA_TYPE_DOUBLE;
        default: return VG_NITF_ENGRDA_TYPE_UNKNOWN;
        }
    case 'C':
      switch (this->DataTypeSize)
        {
        case 4: return VG_NITF_ENGRDA_TYPE_COMPLEX_FLOAT;
        case 8: return VG_NITF_ENGRDA_TYPE_COMPLEX_DOUBLE;
        default: return VG_NITF_ENGRDA_TYPE_UNKNOWN;
        }
    default: return VG_NITF_ENGRDA_TYPE_UNKNOWN;
    }
}

//----------------------------------------------------------------------------
void* vtkVgNitfEngrdaElement::GetData(void)
{
  return this->Data;
}

//----------------------------------------------------------------------------
bool vtkVgNitfEngrdaElement::GetData(std::string& value)
{
  if (this->Type != 'A')
    {
    return false;
    }
  std::stringstream ss;
  ss.write(this->Data, this->DataCount);
  value = ss.str();
  return true;
}

//----------------------------------------------------------------------------
std::string& vtkVgNitfEngrdaElement::GetUnits(void)
{
  return this->DataUnits;
}

//----------------------------------------------------------------------------
vtkVgNitfEngrda::vtkVgNitfEngrda()
{
}

//----------------------------------------------------------------------------
bool vtkVgNitfEngrda::Parse(const char* data, size_t len)
{
  size_t bytesRemaining = len;

  // Parse engrda metadata from a given data buffer
  std::stringstream block;
  block.write(data, len);
  if (!block)
    {
    return false;
    }

    {
    // Read the resource name
    char buf[21];
    memset(buf, 0, 21);
    block.read(buf, 20);
    bytesRemaining -= 20;
    this->Resource = buf;
    }

  size_t recnt;
    {
    // Read the record count
    char buf[4];
    std::memset(buf, 0, 4);
    block.read(buf, 3);
    bytesRemaining -= 3;
    if (!block)
      {
      return false;
      }
    std::stringstream ssconv;
    ssconv << buf;
    ssconv >> recnt;
    }

  // Read each record
  for (size_t i = 0; i < recnt; ++i)
    {
    vtkVgNitfEngrdaElementRefPtr element = new vtkVgNitfEngrdaElement;
      {
      // 1: Parse the label length
      size_t ln;
      char bufln[3];
      std::memset(bufln, 0, 3);
      block.read(bufln, 2);
      bytesRemaining -= 2;
      if (!block)
        {
        return false;
        }
      std::stringstream ssconv(bufln);
      ssconv >> ln;
      if (!ssconv || bytesRemaining < ln)
        {
        return false;
        }
      // 2: Parse the label
      char* buf_lbl = new char[ln + 1];
      std::memset(buf_lbl, 0, ln + 1);
      block.read(buf_lbl, ln);
      bytesRemaining -= ln;
      if (!block)
        {
        delete[] buf_lbl;
        return false;
        }
      element->Label = buf_lbl;
      delete[] buf_lbl;
      }
      {
      // 3: Parse the matrix column count
      char bufmtxc[5];
      std::memset(bufmtxc, 0, 5);
      block.read(bufmtxc, 4);
      bytesRemaining -= 4;
      if (!block)
        {
        return false;
        }
      std::stringstream ssconv(bufmtxc);
      ssconv >> element->MatrixColumnCount;
      if (!ssconv) return false;
      }
      {
      // 4: Parse the matrix row count
      char bufmtxr[5];
      std::memset(bufmtxr, 0, 5);
      block.read(bufmtxr, 4);
      bytesRemaining -= 4;
      if (!block)
        {
        return false;
        }
      std::stringstream ssconv(bufmtxr);
      ssconv >> element->MatrixRowCount;
      if (!ssconv)
        {
        return false;
        }
      }
      {
      // 5: Parse the data type
      block.read(&element->Type, 1);
      bytesRemaining--;
      if (!block)
        {
        return false;
        }
      }
      {
      // 6: Parse the data type size
      char bufdts[2];
      std::memset(bufdts, 0, 2);
      block.read(bufdts, 1);
      bytesRemaining--;
      if (!block)
        {
        return false;
        }
      std::stringstream ssconv(bufdts);
      ssconv >> element->DataTypeSize;
      if (!ssconv)
        {
        return false;
        }
      }
      {
      // 7: Parse the data units
      char bufdatu[3];
      std::memset(bufdatu, 0, 3);
      block.read(bufdatu, 2);
      bytesRemaining -= 2;
      if (!block)
        {
        return false;
        }
      element->DataUnits = bufdatu;
      }
      {
      // 8: Parse the data count
      char bufdatc[9];
      std::memset(bufdatc, 0, 9);
      block.read(bufdatc, 8);
      bytesRemaining -= 8;
      if (!block)
        {
        return false;
        }
      std::stringstream ssconv(bufdatc);
      ssconv >> element->DataCount;
      if (!ssconv)
        {
        return false;
        }
      }
      {
      // 9: Parse the data
      size_t len = element->DataTypeSize * element->DataCount;
      if (bytesRemaining < len)
        {
        return false;
        }
      element->Data = new char[len];
      std::memset(element->Data, 0, len);
      block.read(element->Data, len);
      if (!block)
        {
        return false;
        }
      }
    this->Elements[element->Label] = element;
    }
  return true;
}

//----------------------------------------------------------------------------
const std::string& vtkVgNitfEngrda ::GetSource(void) const
{
  return this->Resource;
}

//----------------------------------------------------------------------------
const vtkVgNitfEngrdaElementRefPtr
vtkVgNitfEngrda::Get(const std::string& label) const
{
  vtkVgNitfEngrda::const_iterator i;
  i = this->Elements.find(label);
  if (i == this->Elements.end())
    {
    return NULL;
    }
  return i->second;
}

//----------------------------------------------------------------------------
vtkVgNitfEngrda::const_iterator vtkVgNitfEngrda ::Begin(void) const
{
  return this->Elements.begin();
}

//----------------------------------------------------------------------------
vtkVgNitfEngrda::const_iterator vtkVgNitfEngrda::End(void) const
{
  return this->Elements.end();
}
