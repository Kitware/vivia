/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsApplication_h
#define __vsApplication_h

#include <QApplication>
#include <QUrl>

#include <qtGlobal.h>

#include <vgExport.h>

class QCloseEvent;

class vsCore;
class vsMainWindow;
class vsUiExtensionInterface;

class vsApplicationPrivate;

class VSP_USERINTERFACE_EXPORT vsApplication : public QApplication
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

  void initialize();

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
