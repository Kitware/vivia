/*=========================================================================

   Program: ParaView
   Module:    pqCoreTestUtility.h

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

#ifndef _pqCoreTestUtility_h
#define _pqCoreTestUtility_h

#include "pqTestUtility.h"

#include <vgExport.h>

class QAction;
class QMainWindow;
class QMenu;
class QMenuBar;

class qtCliArgs;

class QVTKWidget;
class vtkRenderWindow;

/// Provides ParaView-specific functionality for regression testing
class QT_TESTINGSUPPORT_EXPORT pqCoreTestUtility : public pqTestUtility
{
  Q_OBJECT
  typedef pqTestUtility Superclass;

public:
  pqCoreTestUtility(QMainWindow* window, QVTKWidget* viewport = 0,
                    QObject* parent = 0);
  ~pqCoreTestUtility();

public:
  /// Set environment variable containing data root (e.g. $VISGUI_DATA_ROOT).
  /// This will also be used as a replacement "tag" in file open dialog paths.
  void SetDataRootEnvironment(const QString& env);
  QString GetDataRootEnvironment();

  /// Set environment variable containing test root (e.g. $TEST_TEMP_ROOT).
  /// This will also be used as a replacement "tag" in file open dialog paths.
  void SetTestTempRootEnvironment(const QString& env);
  QString GetTestTempRootEnvironment();

  /// Set the absolute path to the data root directory
  void SetDataRoot(const QString& root);
  QString GetDataRoot();

  /// Set the absolute path to the data root directory
  QString GetTestTempRoot();

  /// Sets the temporary test directory in which tests can write
  /// temporary outputs, difference images etc.
  void SetTestDirectory(const QString& dir);
  QString GetTestDirectory();

  /// Sets the directory where baseline images are stored.
  void SetBaselineDirectory(const QString& dir);

  /// Compares the contents of a render window to a reference image,
  /// returning true iff the two match within a given threshold
  bool CompareImage(vtkRenderWindow* RenderWindow,
                    const QString& ReferenceImage,
                    double Threshold,
                    std::ostream& Output);

  /// Adds a standard testing menu to supplied \p menubar. If \p before is
  /// specified, the menu will be added before \p before; otherwise it is added
  /// to the end.
  void AddTestingMenu(QMenuBar* menuBar, QMenu* before = 0);

  /// Adds a standard testing menu to supplied \p menubar. If \p before is
  /// specified, the menu will be added before \p before; otherwise it is added
  /// to the end. The menu will have an accelerator that is the first character
  /// in \p availableAccelerators that is also in the menu name.
  void AddTestingMenu(QMenuBar* menuBar,
                      const QString& availableAccelerators,
                      QMenu* before = 0);

  /// Adds a standard testing menu as a sub-menu of the supplied \p menu. If
  /// \p before is specified, the menu will be added before \p before;
  /// otherwise it is added to the end.
  void AddTestingMenu(QMenu* menu, QAction* before = 0);

  /// Adds a standard testing menu as a sub-menu of the supplied \p menu. If
  /// \p before is specified, the menu will be added before \p before;
  /// otherwise it is added to the end. The menu will have an accelerator that
  /// is the first character in \p availableAccelerators that is also in the menu
  /// name.
  void AddTestingMenu(QMenu* menu,
                      const QString& availableAccelerators,
                      QAction* before = 0);

  bool PlayTest(const QString& filename);
  bool PlayTests(const QStringList& filenames);
  void RecordTest(const QString& filename);

  bool IsActive()
    { return this->Active; }

  /// Parse application command line and remove test-specific arguments.
  void ParseCommandLine(QStringList& args);

  /// Parse CLI options from qtCliArgs.
  void ParseCommandLine(const qtCliArgs& args);

  /// Run any test scripts discovered by ParseCommandLine().
  void ProcessCommandLine();

  /// Register command line options with qtCliArgs.
  static void AddCommandLineOptions(qtCliArgs& args);

signals:
  void StartedTesting();
  void StoppedTesting();

protected slots:
  void OnPlayTest();
  void OnRecordTest();
  void OnRecordingStopped();

protected:
  bool TakeArgValue(QMutableStringListIterator& iter, QString& value);
  void SetActive(bool active);
  void SetViewportSize();

  // Helper function.
  QMenu* CreateTestMenu(const QString& availableAccelerators,
                        QWidget* parent = 0);

private:
  QString DataRoot;
  QString DataRootEnvironment;
  QString TestDirectory;
  QString BaselineDirectory;

  QString TestTempRoot;
  QString TestTempRootEnvironment;
  QString TestFileName;
  QString TestBaselineFileName;

  QMainWindow* Window;
  QVTKWidget* Viewport;

  bool Active;
  bool ExitOnTestComplete;

  class MakeActive;
  class FixViewportSize;

  QScopedPointer<MakeActive> ActiveMaker;
  QScopedPointer<FixViewportSize> ViewportSizeFixer;
};

#endif // !_pqCoreTestUtility_h
