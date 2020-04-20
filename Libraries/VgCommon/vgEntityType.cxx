/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgEntityType.h"

#include <string.h>

namespace
{

//-----------------------------------------------------------------------------
inline void SetColorInternal(double (&arr)[3], double r, double g, double b)
{
  arr[0] = r;
  arr[1] = g;
  arr[2] = b;
}

//-----------------------------------------------------------------------------
inline void GetColorInternal(const double (&arr)[3],
                             double& r, double& g, double& b)
{
  r = arr[0];
  g = arr[1];
  b = arr[2];
}

} // end anonymous namespace

//-----------------------------------------------------------------------------
vgEntityType::vgEntityType() :
  HasSecondaryColor(false), UseRandomColors(false), IconIndex(-1),
  IsUsed(false)
{
  Name[0] = '\0';
  this->Color[0] = this->Color[1] = this->Color[2] = 1.0;
  this->SecondaryColor[0] = 1.0;
  this->SecondaryColor[1] = 1.0;
  this->SecondaryColor[2] = 1.0;
  this->LabelForegroundColor[0] = 0.0;
  this->LabelForegroundColor[1] = 0.0;
  this->LabelForegroundColor[2] = 0.0;
  this->LabelBackgroundColor[0] = 1.0;
  this->LabelBackgroundColor[1] = 1.0;
  this->LabelBackgroundColor[2] = 1.0;
}

#ifdef _WIN32
#pragma warning(disable:4996) // 'strncpy' unsafe
#endif

//-----------------------------------------------------------------------------
void vgEntityType::SetName(const char* name)
{
  strncpy(this->Name, name, sizeof(this->Name) - 1);
  this->Name[sizeof(this->Name) - 1] = '\0';
}

//-----------------------------------------------------------------------------
void vgEntityType::SetColor(const double* color)
{ SetColorInternal(this->Color, color[0], color[1], color[2]); }

//-----------------------------------------------------------------------------
void vgEntityType::SetColor(double r, double g, double b)
{ SetColorInternal(this->Color, r, g, b); }

//-----------------------------------------------------------------------------
void vgEntityType::GetColor(double& r, double& g, double& b) const
{ GetColorInternal(this->Color, r, g, b); }

//-----------------------------------------------------------------------------
const double* vgEntityType::GetColor() const
{ return this->Color; }

//-----------------------------------------------------------------------------
void vgEntityType::SetSecondaryColor(double r, double g, double b)
{ SetColorInternal(this->SecondaryColor, r, g, b); }

//-----------------------------------------------------------------------------
void vgEntityType::GetSecondaryColor(double& r, double& g, double& b) const
{ GetColorInternal(this->SecondaryColor, r, g, b); }

//-----------------------------------------------------------------------------
void vgEntityType::SetLabelForegroundColor(const double* color)
{ SetColorInternal(this->LabelForegroundColor, color[0], color[1], color[2]); }

//-----------------------------------------------------------------------------
void vgEntityType::SetLabelForegroundColor(double r, double g, double b)
{ SetColorInternal(this->LabelForegroundColor, r, g, b); }

//-----------------------------------------------------------------------------
void vgEntityType::GetLabelForegroundColor(double& r, double& g, double& b) const
{ GetColorInternal(this->LabelForegroundColor, r, g, b); }

//-----------------------------------------------------------------------------
void vgEntityType::SetLabelBackgroundColor(const double* color)
{ SetColorInternal(this->LabelBackgroundColor, color[0], color[1], color[2]); }

//-----------------------------------------------------------------------------
void vgEntityType::SetLabelBackgroundColor(double r, double g, double b)
{ SetColorInternal(this->LabelBackgroundColor, r, g, b); }

//-----------------------------------------------------------------------------
void vgEntityType::GetLabelBackgroundColor(double& r, double& g, double& b) const
{ GetColorInternal(this->LabelBackgroundColor, r, g, b); }
