/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
