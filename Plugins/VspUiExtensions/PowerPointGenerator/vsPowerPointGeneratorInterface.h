/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsPowerPointGeneratorInterface_h
#define __vsPowerPointGeneratorInterface_h

#include <QObject>
#include <QSharedPointer>

#include <qtGlobal.h>

class vsCore;
class vsMainWindow;
class vsScene;
class vsVideoSource;

class vsPowerPointGeneratorInterfacePrivate;

class vsPowerPointGeneratorInterface : public QObject
{
  Q_OBJECT

public:
  vsPowerPointGeneratorInterface(vsMainWindow*, vsScene*, vsCore*);
  virtual ~vsPowerPointGeneratorInterface();

protected:
  void generatePowerPoint(
    const QString& path, const QString& templateFile, bool generateVideo);

protected slots:
  void setVideoSource(vsVideoSource*);

  void generatePowerPoint();
  void configurePowerPointGeneration();

protected:
  QTE_DECLARE_PRIVATE_SPTR(vsPowerPointGeneratorInterface)

private:
  QTE_DECLARE_PRIVATE(vsPowerPointGeneratorInterface)
  Q_DISABLE_COPY(vsPowerPointGeneratorInterface)
};

#endif
