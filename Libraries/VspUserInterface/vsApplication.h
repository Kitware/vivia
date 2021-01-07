// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsApplication_h
#define __vsApplication_h

#include <QUrl>

#include <vgApplication.h>

#include <qtGlobal.h>

#include <vgExport.h>

class QCloseEvent;

class qtCliArgs;

class vsCore;
class vsMainWindow;
class vsUiExtensionInterface;

class vsApplicationPrivate;

class VSP_USERINTERFACE_EXPORT vsApplication : public vgApplication
{
  Q_OBJECT

public:
  vsApplication(int& argc, char** argv);
  virtual ~vsApplication();

  static vsApplication* instance();
  static QList<vsUiExtensionInterface*> uiExtensions();

  vsMainWindow* firstView() const;
  vsMainWindow* newView(vsMainWindow* invokingView = 0);
  virtual void connectSource(const QString& identifier, const QUrl& uri);

  void initialize(const qtCliArgs&);

signals:
  void lastViewClosed();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsApplication)
  friend class vsMainWindow;

  void viewCloseEvent(vsMainWindow*, QCloseEvent*);

private:
  QTE_DECLARE_PRIVATE(vsApplication)
  Q_DISABLE_COPY(vsApplication)
};

#endif
