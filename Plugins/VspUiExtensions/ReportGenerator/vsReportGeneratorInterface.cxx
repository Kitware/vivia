// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsReportGeneratorInterface.h"

#include <QMenu>
#include <QMessageBox>

#include <qtPrioritizedMenuProxy.h>

#include <vgFileDialog.h>

#include <vvGenerateReportDialog.h>

#include <vsCore.h>
#include <vsEventUserInfo.h>
#include <vsMainWindow.h>
#include <vsScene.h>

#include "vsKmlWriter.h"
#include "vsReportGenerator.h"

QTE_IMPLEMENT_D_FUNC(vsReportGeneratorInterface)

//-----------------------------------------------------------------------------
class vsReportGeneratorInterfacePrivate
{
public:
  explicit vsReportGeneratorInterfacePrivate(vsMainWindow* window,
                                             vsScene* scene, vsCore* core);

  vsCore* const Core;
  vsScene* const Scene;
  vsMainWindow* const Window;
  vsVideoSource* VideoSource;
};

//-----------------------------------------------------------------------------
vsReportGeneratorInterfacePrivate::vsReportGeneratorInterfacePrivate(
  vsMainWindow* window, vsScene* scene, vsCore* core) :
  Core(core),
  Scene(scene),
  Window(window),
  VideoSource(0)
{
}

//-----------------------------------------------------------------------------
vsReportGeneratorInterface::vsReportGeneratorInterface(
  vsMainWindow* window, vsScene* scene, vsCore* core) :
  QObject(window),
  d_ptr(new vsReportGeneratorInterfacePrivate(window, scene, core))
{
  qtPrioritizedMenuProxy* menu = window->toolsMenu();

#ifdef ENABLE_KML_EXPORT
  QAction* const actionExportKml = new QAction("Export KML...", this);
  menu->insertAction(actionExportKml, 600);

  connect(actionExportKml, SIGNAL(triggered()),
          this, SLOT(exportKml()));
#endif

#ifdef ENABLE_REPORT_GENERATION
  QAction* const actionGenerateReport =
    new QAction("Generate Report...", this);
  menu->insertAction(actionGenerateReport, 640);

  connect(actionGenerateReport, SIGNAL(triggered()),
          this, SLOT(generateReport()));
#endif

  connect(core, SIGNAL(videoSourceChanged(vsVideoSource*)),
          this, SLOT(setVideoSource(vsVideoSource*)));
  this->setVideoSource(core->videoSource());
}

//-----------------------------------------------------------------------------
vsReportGeneratorInterface::~vsReportGeneratorInterface()
{
}

//-----------------------------------------------------------------------------
void vsReportGeneratorInterface::setVideoSource(vsVideoSource* vs)
{
  QTE_D(vsReportGeneratorInterface);
  d->VideoSource = vs;
}

//-----------------------------------------------------------------------------
void vsReportGeneratorInterface::generateReport()
{
  QTE_D(vsReportGeneratorInterface);

  if (!d->VideoSource)
    {
    QMessageBox::critical(d->Window, QString(), "No video loaded.");
    return;
    }

  vvGenerateReportDialog dlg(d->Window);
  if (dlg.exec() == QDialog::Accepted)
    {
    this->generateReport(dlg.outputPath(), dlg.generateVideo());
    }
}

//-----------------------------------------------------------------------------
void vsReportGeneratorInterface::exportKml()
{
  QTE_D(vsReportGeneratorInterface);

  if (!d->VideoSource)
    {
    QMessageBox::critical(d->Window, QString(), "No video loaded.");
    return;
    }

  QString path = vgFileDialog::getSaveFileName(
                   d->Window, "KML export location", QString(),
                   "KML File (*.kml);;");

  if (!path.isEmpty())
    {
    this->exportKml(path);
    }
}

#ifdef ENABLE_REPORT_GENERATION
//-----------------------------------------------------------------------------
void vsReportGeneratorInterface::generateReport(
  const QString& path, bool generateVideo)
{
  QTE_D(vsReportGeneratorInterface);

  vsReportGenerator rg(d->Scene->eventList(),
                       d->Core->eventTypeRegistry(),
                       d->VideoSource);
  rg.setOutputPath(path);
  rg.generateReport(generateVideo);
}
#else
void vsReportGeneratorInterface::generateReport(
  const QString& path, bool generateVideo)
{
}
#endif

//-----------------------------------------------------------------------------
void vsReportGeneratorInterface::exportKml(const QString& path)
{
  QTE_D(vsReportGeneratorInterface);

  vsKmlWriter kmlWriter(d->Scene->eventList(),
                        d->Core->eventTypeRegistry(),
                        d->VideoSource);
  kmlWriter.setOutputPath(path);
  kmlWriter.write();
}
