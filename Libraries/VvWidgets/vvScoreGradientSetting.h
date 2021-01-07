// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvScoreGradientSetting_h
#define __vvScoreGradientSetting_h

#include <qtAbstractSetting.h>

#include <vgExport.h>

class VV_WIDGETS_EXPORT vvScoreGradientSetting : public qtAbstractSetting
{
public:
  virtual qtSettings::Scope scope() const;

  virtual bool isModified();
  virtual void commit(QSettings& store);

protected:
  virtual void initialize(const QSettings& store);
  virtual QString key() const;
};

#endif
