// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsPowerPointGeneratorInterface.h"

#include <QMenu>
#include <QMessageBox>
#include <QDateTime>

#include <qtPrioritizedMenuProxy.h>
#include <qtPrioritizedToolBarProxy.h>

#include <vgFileDialog.h>

#include <vvGeneratePowerPointDialog.h>

#include <vsCore.h>
#include <vsEventUserInfo.h>
#include <vsMainWindow.h>
#include <vsScene.h>

#include "vsPowerPointGenerator.h"

QTE_IMPLEMENT_D_FUNC(vsPowerPointGeneratorInterface)

//-----------------------------------------------------------------------------
class vsPowerPointGeneratorInterfacePrivate
{
public:
  explicit vsPowerPointGeneratorInterfacePrivate(vsMainWindow* window,
                                                 vsScene* scene, vsCore* core);

  vsCore* const Core;
  vsScene* const Scene;
  vsMainWindow* const Window;
  vsVideoSource* VideoSource;
};

//-----------------------------------------------------------------------------
vsPowerPointGeneratorInterfacePrivate::vsPowerPointGeneratorInterfacePrivate(
  vsMainWindow* window, vsScene* scene, vsCore* core) :
  Core(core),
  Scene(scene),
  Window(window),
  VideoSource(0)
{
}

//-----------------------------------------------------------------------------
vsPowerPointGeneratorInterface::vsPowerPointGeneratorInterface(
  vsMainWindow* window, vsScene* scene, vsCore* core) :
  QObject(window),
  d_ptr(new vsPowerPointGeneratorInterfacePrivate(window, scene, core))
{
  QAction* const actionConfigurePowerPointGeneration =
    new QAction("Configure PowerPoint Generation...", this);
  QAction* const actionGeneratePowerPoint =
    new QAction("Generate PowerPoint", this);

  qtPrioritizedMenuProxy* menu = window->toolsMenu();
  menu->insertAction(actionConfigurePowerPointGeneration, 680);
  menu->insertAction(actionGeneratePowerPoint, 680);

  qtPrioritizedToolBarProxy* toolBar = window->toolsToolBar();
  toolBar->insertAction(actionGeneratePowerPoint, 680);

  connect(actionConfigurePowerPointGeneration, SIGNAL(triggered()),
    this, SLOT(configurePowerPointGeneration()));
  connect(actionGeneratePowerPoint, SIGNAL(triggered()),
    this, SLOT(generatePowerPoint()));

  connect(core, SIGNAL(videoSourceChanged(vsVideoSource*)),
          this, SLOT(setVideoSource(vsVideoSource*)));

  this->setVideoSource(core->videoSource());
}

//-----------------------------------------------------------------------------
vsPowerPointGeneratorInterface::~vsPowerPointGeneratorInterface()
{
}

//-----------------------------------------------------------------------------
void vsPowerPointGeneratorInterface::setVideoSource(vsVideoSource* vs)
{
  QTE_D(vsPowerPointGeneratorInterface);
  d->VideoSource = vs;
}

//-----------------------------------------------------------------------------
void vsPowerPointGeneratorInterface::configurePowerPointGeneration()
{
  QTE_D(vsPowerPointGeneratorInterface);

  vvGeneratePowerPointDialog dlg(d->Window);
  dlg.exec();
}

//-----------------------------------------------------------------------------
void vsPowerPointGeneratorInterface::generatePowerPoint()
{
  QTE_D(vsPowerPointGeneratorInterface);

  if (!d->VideoSource)
    {
    QMessageBox::critical(d->Window, QString(), "No video loaded.");
    return;
    }

  vvGeneratePowerPointDialog dlg(d->Window);
  if (!dlg.getLastConfig())
    {
    return;
    }

  generatePowerPoint(dlg.outputPath(), dlg.templateFile(), dlg.generateVideo());
}

//-----------------------------------------------------------------------------
void vsPowerPointGeneratorInterface::generatePowerPoint(
  const QString& path, const QString& templateFile, bool generateVideo)
{
  QTE_D(vsPowerPointGeneratorInterface);

  const QDateTime& now = QDateTime::currentDateTime();
  const QString finalPath =
    path + '/' + now.toString("yyyy-MM-dd/hh.mm.ss.zzz");

  if (!QDir().mkpath(finalPath))
    {
    QString msg("Unable to create output directory \"%1\".");
    QMessageBox::critical(d->Window, QString(), msg.arg(finalPath));
    return;
    }

  vsPowerPointGenerator rg(d->Scene->eventList(), d->Scene->trackList(),
                           d->Core->eventTypeRegistry(), d->VideoSource);

  rg.setOutputPath(finalPath);
  rg.setTemplateFile(templateFile);
  rg.generatePowerPoint(generateVideo);
}
