/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqSettings_h
#define __vqSettings_h

#include <QUrl>
#include <QList>

#include <qtSettings.h>

#include <vvDescriptorStyle.h>
#include <vvScoreGradient.h>

class vqSettings : public qtSettings
{
public:
  vqSettings();

  qtSettings_declare(QUrl, queryServerUri, setQueryServerUri);
  qtSettings_declare(QUrl, queryVideoUri, setQueryVideoUri);
  qtSettings_declare(QUrl, queryCacheUri, setQueryCacheUri);
  qtSettings_declare(QUrl, predefinedQueryUri, setPredefinedQueryUri);
  qtSettings_declare(QList<QUrl>, videoProviders, setVideoProviders);
  qtSettings_declare(int, resultPageCount, setResultPageCount);
  qtSettings_declare(double, resultClipPadding, setResultClipPadding);
  qtSettings_declare(vvScoreGradient, scoreGradient, setScoreGradient);
  qtSettings_declare(int, iqrWorkingSetSize, setIqrWorkingSetSize);
  qtSettings_declare(int, iqrRefinementSetSize, setIqrRefinementSetSize);
  qtSettings_declare(vvDescriptorStyle::Map,
                     descriptorStyles, setDescriptorStyles);
};

#endif
