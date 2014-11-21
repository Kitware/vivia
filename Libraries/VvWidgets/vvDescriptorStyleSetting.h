/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvDescriptorStyleSetting_h
#define __vvDescriptorStyleSetting_h

#include <qtAbstractSetting.h>

#include <vgExport.h>

namespace vvDescriptorStyle
{

class VV_WIDGETS_EXPORT Setting : public ::qtAbstractSetting
{
public:
  virtual qtSettings::Scope scope() const;

  virtual void commit(QSettings& store);

protected:
  virtual void initialize(const QSettings& store);
  virtual QString key() const;
};

} // namespace vvDescriptorStyle

#endif
