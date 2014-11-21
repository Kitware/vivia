/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
