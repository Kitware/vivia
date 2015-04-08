/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqSettings.h"

#define QTSETTINGS_SUBCLASS_NAME vqSettings
#include <qtSettingsImpl.h>

#include <vvDescriptorStyleSetting.h>
#include <vvScoreGradientSetting.h>

static const char* const keyQueryServerUri         = "QueryServerUri";
static const char* const keyQueryVideoUri          = "QueryVideoUri";
static const char* const keyQueryCacheUri          = "QueryCacheUri";
static const char* const keyVideoProviderUris      = "VideoProviderUris";
static const char* const keyPredefinedQueryUri     = "PredefinedQueryUri";
static const char* const keyResultPageCount        = "ResultPageCount";
static const char* const keyResultClipPadding      = "ResultClipPadding";
static const char* const keyScoreGradient          = "ScoreGradient";
static const char* const keyIqrWorkingSetSize      = "IqrWorkingSetSize";
static const char* const keyIqrRefinementSetSize   = "IqrRefinementSetSize";
static const char* const keyDescriptorStyles       = "DescriptorStyles";

//-----------------------------------------------------------------------------
vqSettings::vqSettings()
{
  this->declareSetting(keyQueryServerUri);
  this->declareSetting(keyQueryVideoUri);
  this->declareSetting(keyQueryCacheUri);
  this->declareSetting(keyVideoProviderUris);
  this->declareSetting(keyPredefinedQueryUri);
  this->declareSetting(keyResultPageCount, 100);
  this->declareSetting(keyResultClipPadding, 2.0);
  this->declareSetting(keyScoreGradient, new vvScoreGradientSetting);
  this->declareSetting(keyIqrWorkingSetSize, 1500);
  this->declareSetting(keyIqrRefinementSetSize, 10);
  this->declareSetting(keyDescriptorStyles, new vvDescriptorStyle::Setting);
}

//-----------------------------------------------------------------------------
qtSettings_implement(QUrl, queryServerUri, QueryServerUri)
qtSettings_implement(QUrl, queryVideoUri, QueryVideoUri)
qtSettings_implement(QUrl, queryCacheUri, QueryCacheUri)
qtSettings_implement(QUrl, predefinedQueryUri, PredefinedQueryUri)
qtSettings_implement(int, resultPageCount, ResultPageCount)
qtSettings_implement(double, resultClipPadding, ResultClipPadding)
qtSettings_implement(vvScoreGradient, scoreGradient, ScoreGradient)
qtSettings_implement(int, iqrWorkingSetSize, IqrWorkingSetSize)
qtSettings_implement(int, iqrRefinementSetSize, IqrRefinementSetSize)

//-----------------------------------------------------------------------------
QList<QUrl> vqSettings::videoProviders() const
{
  QList<QUrl> result;
  QVariant value = this->value(keyVideoProviderUris);
  if (value.canConvert(QVariant::List))
    {
    foreach (const QVariant& v, value.toList())
      result.append(v.toUrl());
    }
  else if (value.canConvert(QVariant::Url))
    {
    result.append(value.toUrl());
    }
  return result;
}

//-----------------------------------------------------------------------------
void vqSettings::setVideoProviders(QList<QUrl> providers)
{
  QList<QVariant> list;
  foreach (const QUrl& uri, providers)
    list.append(uri);
  this->setValue(keyVideoProviderUris, list);
}

//-----------------------------------------------------------------------------
vvDescriptorStyle::Map vqSettings::descriptorStyles() const
{
  QVariant value = this->value(keyDescriptorStyles);
  if (value.canConvert(QVariant::Hash))
    return vvDescriptorStyle::Map(value.toHash());
  return vvDescriptorStyle::Map();
}

//-----------------------------------------------------------------------------
void vqSettings::setDescriptorStyles(vvDescriptorStyle::Map styles)
{
  this->setValue(keyDescriptorStyles, styles.serializable());
}
