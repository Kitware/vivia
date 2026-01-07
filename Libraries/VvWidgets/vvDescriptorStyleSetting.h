// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
