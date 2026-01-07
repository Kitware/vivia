// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsPowerPointGenerator_h
#define __vsPowerPointGenerator_h

#include <vgVtkVideoFrame.h>

#include <qtGlobal.h>

#include <QSharedPointer>

class vtkVgEventTypeRegistry;
class vtkVgTrack;

class vsEventUserInfo;
class vsVideoSource;

class vsPowerPointGeneratorPrivate;

class vsPowerPointGenerator
{
public:
  vsPowerPointGenerator(QList<vsEventUserInfo> events,
                        QList<vtkVgTrack*> tracks,
                        vtkVgEventTypeRegistry* registry,
                        vsVideoSource* videoSource);
  virtual ~vsPowerPointGenerator();

  void setOutputPath(const QString& path);
  void setTemplateFile(const QString& templateFile);

  void generatePowerPoint(bool generateVideo);

protected:
  QTE_DECLARE_PRIVATE_SPTR(vsPowerPointGenerator)

private:
  QTE_DECLARE_PRIVATE(vsPowerPointGenerator)
  Q_DISABLE_COPY(vsPowerPointGenerator)
};

#endif
