// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
