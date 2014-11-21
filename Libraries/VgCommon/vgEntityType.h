/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgEntityType_h
#define __vgEntityType_h

#include <vgExport.h>

class VG_COMMON_EXPORT vgEntityType
{
public:
  vgEntityType();

  void SetName(const char* name);
  const char* GetName() const { return this->Name; }

  void SetColor(double r, double g, double b);
  void GetColor(double& r, double& g, double& b) const;

  void SetHasSecondaryColor(bool enable) { this->HasSecondaryColor = enable; }
  bool GetHasSecondaryColor() const      { return this->HasSecondaryColor; }

  void SetSecondaryColor(double r, double g, double b);
  void GetSecondaryColor(double& r, double& g, double& b) const;

  void SetLabelForegroundColor(double r, double g, double b);
  void GetLabelForegroundColor(double& r, double& g, double& b) const;

  void SetLabelBackgroundColor(double r, double g, double b);
  void GetLabelBackgroundColor(double& r, double& g, double& b) const;

  void SetUseRandomColors(bool enable) { this->UseRandomColors = enable; }
  bool GetUseRandomColors() const      { return this->UseRandomColors; }

  void SetIconIndex(int index) { this->IconIndex = index; }
  int  GetIconIndex() const    { return this->IconIndex; }

  void SetEntityTypeInfo(const vgEntityType& et)
    {
    *this = et;
    }

  void SetIsUsed(bool used) { this->IsUsed = used; }
  bool GetIsUsed() const    { return this->IsUsed; }

protected:
  char Name[256];

  double Color[3];
  double SecondaryColor[3];
  double LabelForegroundColor[3];
  double LabelBackgroundColor[3];

  bool HasSecondaryColor;
  bool UseRandomColors;

  int IconIndex;

  bool IsUsed;
};

#endif // __vgEntityType_h
