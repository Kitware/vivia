/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
