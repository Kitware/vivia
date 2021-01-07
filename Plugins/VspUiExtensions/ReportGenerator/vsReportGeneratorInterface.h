// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsReportGeneratorInterface_h
#define __vsReportGeneratorInterface_h

#include <QObject>
#include <QSharedPointer>

#include <qtGlobal.h>

class vsCore;
class vsMainWindow;
class vsScene;
class vsVideoSource;

class vsReportGeneratorInterfacePrivate;

class vsReportGeneratorInterface : public QObject
{
  Q_OBJECT

public:
  vsReportGeneratorInterface(vsMainWindow*, vsScene*, vsCore*);
  virtual ~vsReportGeneratorInterface();

protected:
  void generateReport(const QString& path, bool generateVideo);
  void exportKml(const QString& path);

protected slots:
  void setVideoSource(vsVideoSource*);

  void generateReport();
  void exportKml();

protected:
  QTE_DECLARE_PRIVATE_SPTR(vsReportGeneratorInterface)

private:
  QTE_DECLARE_PRIVATE(vsReportGeneratorInterface)
  Q_DISABLE_COPY(vsReportGeneratorInterface)
};

#endif
