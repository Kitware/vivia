/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsReportGenerator_h
#define __vsReportGenerator_h

#include <QSharedPointer>

#include <qtGlobal.h>

#include <vgVtkVideoFrame.h>

template <typename T> class QList;

class vsEventUserInfo;
class vsVideoSource;

class vsReportGeneratorPrivate;

class vtkVgEventTypeRegistry;

class vsReportGenerator
{
public:
  vsReportGenerator(QList<vsEventUserInfo> events,
                    vtkVgEventTypeRegistry* registry,
                    vsVideoSource* videoSource);
  virtual ~vsReportGenerator();

  void setOutputPath(const QString& path);

  void generateReport(bool generateVideo);

protected:
  QTE_DECLARE_PRIVATE_SPTR(vsReportGenerator)

private:
  QTE_DECLARE_PRIVATE(vsReportGenerator)
  Q_DISABLE_COPY(vsReportGenerator)
};

#endif
