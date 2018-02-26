#ifndef __vpApplication_h
#define __vpApplication_h

#include "vpSettings.h"

#include <vgApplication.h>

class vpApplication : public vgApplication
{
  Q_OBJECT

public:
  vpApplication(int& argc, char** argv) : vgApplication(argc, argv)
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
