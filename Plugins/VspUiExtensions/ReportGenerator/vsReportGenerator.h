// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
