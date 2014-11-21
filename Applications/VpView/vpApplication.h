#ifndef __vpApplication_h
#define __vpApplication_h

// Qt includes.
#include <QApplication>

// VpView includes.
#include <vpSettings.h>

class vpApplication : public QApplication
{
  Q_OBJECT

public:
  vpApplication(int& argc, char** argv) : QApplication(argc, argv)
    {
    }

  virtual ~vpApplication()
    {
    }

signals:
  void settingsChanged(vpSettings::SettingsKeys flag);

public:
  static vpApplication* instance()
    {
    return qobject_cast<vpApplication*>(QApplication::instance());
    }

  void notifySettingsChanged(vpSettings::SettingsKeys key)
    {
    emit this->settingsChanged(key);
    }


};

#endif // __vpApplication_h
