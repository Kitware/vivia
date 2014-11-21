/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
