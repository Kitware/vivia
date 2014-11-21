/*=========================================================================

   Program: ParaView
   Module:    pqCoreTestUtility.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "pqCoreTestUtility.h"

#include "pqFileDialogEventPlayer.h"
#include "pqFileDialogEventTranslator.h"
#include "pqInvokeMethodEventPlayer.h"
#include "pqQDialogButtonBoxEventPlayer.h"
#include "pqQDialogButtonBoxEventTranslator.h"
#include "pqQteDoubleSliderEventPlayer.h"
#include "pqQteDoubleSliderEventTranslator.h"
#include "pqQVTKWidgetEventPlayer.h"
#include "pqQVTKWidgetEventTranslator.h"
#include "pqVgMixerEventPlayer.h"
#include "pqVgMixerEventTranslator.h"
#include "pqXMLEventSource.h"
#include "pqXMLEventObserver.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QString>

#include <qtCliArgs.h>

#include "QtTestingConfigure.h"

#include "QVTKWidget.h"
#include "vtkSmartPointer.h"
#include "vtkRenderWindow.h"
#include "vtkTesting.h"

#include "vtksys/SystemTools.hxx"

//BEGIN helper classes

//-----------------------------------------------------------------------------
class pqCoreTestUtility::MakeActive
{
public:
  MakeActive(pqCoreTestUtility* parent) : Parent(parent)
    {
    this->Parent->SetActive(true);
    }

  ~MakeActive()
    {
    this->Parent->SetActive(false);
    }

private:
  pqCoreTestUtility* Parent;
};

//-----------------------------------------------------------------------------
class pqCoreTestUtility::FixViewportSize
{
public:
  FixViewportSize(pqCoreTestUtility* parent) : Parent(parent)
    {
    // Save the current window state and geometry. We do not want running a
    // test to mess up a layout the user has configured.
    this->State = this->Parent->Window->saveState();
    this->Geometry = this->Parent->Window->saveGeometry();

    this->Parent->SetViewportSize();
    }

  ~FixViewportSize()
    {
    // 'Unlock' the viewport window to allow resizing
    this->Parent->Viewport->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    // Restore the saved state/geometry.
    this->Parent->Window->restoreGeometry(this->Geometry);
    this->Parent->Window->restoreState(this->State);
    }

private:
  QByteArray State;
  QByteArray Geometry;
  pqCoreTestUtility* Parent;
};

//END helper classes

///////////////////////////////////////////////////////////////////////////////

//BEGIN pqCoreTestUtility

//-----------------------------------------------------------------------------
pqCoreTestUtility::pqCoreTestUtility(QMainWindow* window, QVTKWidget* viewport,
                                     QObject* p)
  : pqTestUtility(p), TestTempRootEnvironment("TEST_TEMP_ROOT"),
    Window(window), Viewport(viewport),
    Active(false), ExitOnTestComplete(false)
{
  this->addEventSource("xml", new pqXMLEventSource(this));
  this->addEventObserver("xml", new pqXMLEventObserver(this));

  this->eventTranslator()->addWidgetEventTranslator(
    new pqQVTKWidgetEventTranslator(this));
  this->eventTranslator()->addWidgetEventTranslator(
    new pqQDialogButtonBoxEventTranslator(this));
  this->eventTranslator()->addWidgetEventTranslator(
    new pqQteDoubleSliderEventTranslator(this));
  this->eventTranslator()->addWidgetEventTranslator(
    new pqVgMixerEventTranslator(this));
  this->eventTranslator()->addWidgetEventTranslator(
    new pqFileDialogEventTranslator(this, this));

  this->eventPlayer()->addWidgetEventPlayer(
    new pqInvokeMethodEventPlayer(this));
  this->eventPlayer()->addWidgetEventPlayer(
    new pqQVTKWidgetEventPlayer(this));
  this->eventPlayer()->addWidgetEventPlayer(
    new pqQDialogButtonBoxEventPlayer(this));
  this->eventPlayer()->addWidgetEventPlayer(
    new pqQteDoubleSliderEventPlayer(this));
  this->eventPlayer()->addWidgetEventPlayer(
    new pqVgMixerEventPlayer(this));
  this->eventPlayer()->addWidgetEventPlayer(
    new pqFileDialogEventPlayer(this, this));

  connect(&this->Translator, SIGNAL(stopped()),
          this, SLOT(OnRecordingStopped()));
}

//-----------------------------------------------------------------------------
pqCoreTestUtility::~pqCoreTestUtility()
{
  // Avoid trying to invoke slot on partly destroyed object (i.e. us); this is
  // necessary as the translator may emit signals when it is destroyed (by our
  // base class destructor)
  disconnect(&this->Translator, 0, this, 0);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::SetDataRootEnvironment(const QString& env)
{
  this->DataRootEnvironment = env;
}

//-----------------------------------------------------------------------------
QString pqCoreTestUtility::GetDataRootEnvironment()
{
  return this->DataRootEnvironment;
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::SetTestTempRootEnvironment(const QString& env)
{
  this->TestTempRootEnvironment = env;
}

//-----------------------------------------------------------------------------
QString pqCoreTestUtility::GetTestTempRootEnvironment()
{
  return this->TestTempRootEnvironment;
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::SetDataRoot(const QString& root)
{
  QString result = root;

  // Let the user override the defaults by setting an environment variable ...
  if (result.isEmpty() && !this->DataRootEnvironment.isEmpty())
    {
    result =
      QString::fromLocal8Bit(qgetenv(qPrintable(this->DataRootEnvironment)));
    }

  // Ensure all slashes face forward ...
  result.replace('\\', '/');

  // Remove any trailing slashes ...
  if (result.size() && result.at(result.size() - 1) == '/')
    {
    result.chop(1);
    }

  // Trim excess whitespace ...
  result = result.trimmed();

  this->DataRoot = result;
}

//-----------------------------------------------------------------------------
QString pqCoreTestUtility::GetDataRoot()
{
  return this->DataRoot;
}

//-----------------------------------------------------------------------------
QString pqCoreTestUtility::GetTestTempRoot()
{
  return this->TestTempRoot;
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::SetTestDirectory(const QString& dir)
{
  this->TestDirectory = dir;
}

//-----------------------------------------------------------------------------
QString pqCoreTestUtility::GetTestDirectory()
{
  return this->TestDirectory;
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::SetBaselineDirectory(const QString& dir)
{
  this->BaselineDirectory = dir;
}

//-----------------------------------------------------------------------------
bool pqCoreTestUtility::CompareImage(vtkRenderWindow* RenderWindow,
                                     const QString& ReferenceImage,
                                     double Threshold, std::ostream& Output)
{
  vtkSmartPointer<vtkTesting> testing = vtkSmartPointer<vtkTesting>::New();
  testing->AddArgument("-D");
  testing->AddArgument(qPrintable(pqCoreTestUtility::BaselineDirectory));
  testing->AddArgument("-T");
  testing->AddArgument(qPrintable(pqCoreTestUtility::TestDirectory));
  testing->AddArgument("-V");
  testing->AddArgument(qPrintable(ReferenceImage));
  testing->SetRenderWindow(RenderWindow);
  return testing->RegressionTest(Threshold, Output) == vtkTesting::PASSED;
}

//-----------------------------------------------------------------------------
bool pqCoreTestUtility::TakeArgValue(QMutableStringListIterator& iter,
                                     QString& value)
{
  if (!iter.hasNext())
    {
    std::cerr << "Not enough arguments." << std::endl;
    return false;
    }

  value = iter.next();
  iter.remove();
  return true;
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::ParseCommandLine(QStringList& args)
{
  QMutableStringListIterator iter(args);

  while (iter.hasNext())
    {
    QString arg = iter.next();

    // check if test script to play was supplied
    if (arg == "--test-script")
      {
      iter.remove();
      if (!this->TakeArgValue(iter, this->TestFileName))
        {
        return;
        }
      // check if test baseline image was supplied
      if (iter.hasNext() && iter.peekNext() == "--test-baseline")
        {
        iter.next();
        iter.remove();
        if (!this->TakeArgValue(iter, this->TestBaselineFileName))
          {
          return;
          }
        }
      }
    // check if data root directory was supplied
    else if (arg == "-D")
      {
      iter.remove();
      if (!this->TakeArgValue(iter, this->DataRoot))
        {
        return;
        }
      }
    // check if directory for test output images was supplied
    else if (arg == "-T")
      {
      iter.remove();
      if (!this->TakeArgValue(iter, this->TestTempRoot))
        {
        return;
        }
      }
    // check if we want to exit after playing test
    else if (arg == "--exit")
      {
      iter.remove();
      this->ExitOnTestComplete = true;
      }
    }
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::ParseCommandLine(const qtCliArgs& args)
{
  this->TestFileName = args.value("test-script");
  this->TestBaselineFileName = args.value("test-baseline");
  this->DataRoot = args.value("test-data-root");
  this->TestTempRoot = args.value("test-temp-root");
  this->ExitOnTestComplete = args.isSet("exit");
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::ProcessCommandLine()
{
  this->SetDataRoot(this->DataRoot);

  if (this->TestFileName.isEmpty())
    {
    return;
    }

  // execute the test script
  bool success = this->PlayTest(this->TestFileName);

  // compare against the baseline render if one was supplied
  if (this->Viewport && !this->TestBaselineFileName.isEmpty())
    {
    // specify directory for output images
    if (this->TestTempRoot.isEmpty())
      {
      this->TestTempRoot = "../../Testing/Temporary";
      }

    this->SetTestDirectory(this->TestTempRoot);

    // 'unlock' the viewport window to allow resizing
    this->Viewport->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    QApplication::processEvents();

    // resize viewport to set the baseline image dimensions
    QSize prevSize = this->Viewport->size();
    this->Viewport->resize(300, 300);

    // \NOTE: This call is mandatory as else tests will fail. Because
    // if not then the image rendered from the last viewport size
    // will be squeezed to fit into 300,300 but will fail to
    // capture updated size of tracks, icons because of this resize.
    // This is to process deferred QT resize events.
    QApplication::processEvents();

    // compare current contents of render window against our baseline
    bool passed =
      this->CompareImage(this->Viewport->GetRenderWindow(),
                         qPrintable(this->TestBaselineFileName),
                         10.0, std::cout);
    success = success && passed;

    this->Viewport->resize(prevSize);
    }

  if (this->ExitOnTestComplete)
    {
    QApplication::instance()->exit(success ? 0 : 1);
    }
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::AddCommandLineOptions(qtCliArgs& args)
{
  qtCliOptions options;
  options.add("test-script <file>", "Execute test '<script>'");
  options.add("test-baseline <image>", "After executing test script, compare"
                                       " view against baseline '<image>'");
  options.add("D").add("test-data-root <dir>",
                       "Use '<dir>' as the base directory for test data files");
  options.add("T").add("test-temp-root <dir>",
                       "Use '<dir>' as the location in which to store"
                       " temporary files related to testing");
  options.add("exit", "Exit the application on completion of the test script");
  args.addOptions(options, "Testing", false);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::AddTestingMenu(QMenuBar* menuBar, QMenu* before)
{
  this->AddTestingMenu(menuBar, "T", before);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::AddTestingMenu(QMenu* menu, QAction* before)
{
  this->AddTestingMenu(menu, "T", before);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::AddTestingMenu(
  QMenuBar* menuBar, const QString& availableAccelerators, QMenu* before)
{
  QMenu* testMenu = this->CreateTestMenu(availableAccelerators, menuBar);
  QAction* beforeAction = before ? before->menuAction() : 0;
  menuBar->insertMenu(beforeAction, testMenu);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::AddTestingMenu(
  QMenu* menu, const QString& availableAccelerators, QAction* before)
{
  QMenu* testMenu = this->CreateTestMenu(availableAccelerators, menu);
  menu->insertMenu(before, testMenu);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::OnPlayTest()
{
  QString filters;
  filters += "XML Files (*.xml);;";
  filters += "All Files (*)";

  QFileDialog fileDialog(0, tr("Play Test"), QString(), filters);
  fileDialog.setObjectName("PlayTestDialog");
  fileDialog.setFileMode(QFileDialog::ExistingFile);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setDefaultSuffix("xml");

  if (fileDialog.exec() == QDialog::Accepted)
    {
    this->PlayTests(fileDialog.selectedFiles());
    }
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::OnRecordTest()
{
  QString filters;
  filters += "XML Files (*.xml);;";
  filters += "All Files (*)";

  QFileDialog fileDialog(0, tr("Record Test"), QString(), filters);
  fileDialog.setObjectName("RecordTestDialog");
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix("xml");

  if (fileDialog.exec() == QDialog::Accepted)
    {
    this->RecordTest(fileDialog.selectedFiles()[0]);
    }
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::OnRecordingStopped()
{
  this->ViewportSizeFixer.reset();
  this->ActiveMaker.reset();
}


//-----------------------------------------------------------------------------
bool pqCoreTestUtility::PlayTest(const QString& filename)
{
  MakeActive ma(this);
  FixViewportSize fvs(this);
  return this->playTests(filename);
}

//-----------------------------------------------------------------------------
bool pqCoreTestUtility::PlayTests(const QStringList& filenames)
{
  MakeActive ma(this);
  FixViewportSize fvs(this);
  return this->playTests(filenames);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::RecordTest(const QString& filename)
{
  this->ActiveMaker.reset(new MakeActive(this));
  this->ViewportSizeFixer.reset(new FixViewportSize(this));
  this->recordTests(filename);
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::SetActive(bool active)
{
  qApp->setProperty("testingActive", active);
  this->Active = active;

  if (active)
    {
    emit this->StartedTesting();
    }
  else
    {
    emit this->StoppedTesting();
    }
}

//-----------------------------------------------------------------------------
void pqCoreTestUtility::SetViewportSize()
{
  // Give the viewport a fixed initial size (and aspect ratio). Since the user
  // can resize the window, we do not know the size of the viewport, but for
  // testing, we want the viewports to be the same initial size and aspect ratio.
  // Changing this size is likely to break tests on one or more platforms.
  this->Viewport->setFixedSize(500, 500);
  this->Window->adjustSize();
}

//-----------------------------------------------------------------------------
QMenu* pqCoreTestUtility::CreateTestMenu(
  const QString& availableAccelerators, QWidget* parent)
{
  QString name = "Test", lname = name.toLower();
  foreach (QChar a, availableAccelerators.toLower())
    {
    int ai = lname.indexOf(a);
    if (ai >= 0)
      {
      name.insert(ai, '&');
      break;
      }
    }
  QMenu* menu = new QMenu(name, parent);
  menu->addAction("&Play Test...", this, SLOT(OnPlayTest()));
  menu->addAction("&Record Test...", this, SLOT(OnRecordTest()));

  return menu;
}

//END pqCoreTestUtility
