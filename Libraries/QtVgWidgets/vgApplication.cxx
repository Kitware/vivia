// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgApplication.h"

#include "vgAboutAction.h"
#include "vgUserManualAction.h"

#include <qtCliArgs.h>
#include <qtColorScheme.h>

#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>
#include <QVariant>

#define WITH_D(type, modifier) \
  for (modifier type* _inst = type::instance(); _inst; _inst = 0) \
    for (modifier type##Private* d = _inst->d_func(); d; d = 0)

#define WITH_VGAPPLICATION_D() WITH_D(vgApplication,)
#define WITH_VGAPPLICATION_D_CONST() WITH_D(vgApplication, const)

namespace // anonymous
{

//-----------------------------------------------------------------------------
QString getProperty(QObject* obj, const char* name, const char* fallback)
{
  const QVariant value = obj->property(name);
  return (value.isNull() ? obj->property(fallback) : value).toString();
}

//-----------------------------------------------------------------------------
bool hasPath(const QFileInfo& fi)
{
  return (fi.path() == "." ? fi.filePath() != fi.fileName() : true);
}

//-----------------------------------------------------------------------------
QString resolveUserManualLocation(const QString& path)
{
  const QFileInfo fi(path);
  if (!fi.isAbsolute())
    {
    if (hasPath(fi))
      {
      // Non-empty relative path specified... according to the API
      // specification, this is undefined behavior; we'll take it to mean
      // relative to the current directory
      return fi.absoluteFilePath();
      }
    else
      {
      QFileInfo aefi(QApplication::applicationFilePath());
      QDir dd(aefi.absolutePath() + "/../doc/user");
      return dd.canonicalPath() + "/" + path;
      }
    }

  return path;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vgApplicationPrivate
{
public:
  QString CopyrightYear;
  QString CopyrightOrganization;
  QString UserManualLocation;
};

QTE_IMPLEMENT_D_FUNC(vgApplication)

//-----------------------------------------------------------------------------
vgApplication::vgApplication(int& argc, char** argv) :
  QApplication(argc, argv), d_ptr(new vgApplicationPrivate)
{
}

//-----------------------------------------------------------------------------
vgApplication::~vgApplication()
{
}

//-----------------------------------------------------------------------------
vgApplication* vgApplication::instance()
{
  return qobject_cast<vgApplication*>(QCoreApplication::instance());
}

//-----------------------------------------------------------------------------
QString vgApplication::copyrightYear()
{
  WITH_VGAPPLICATION_D_CONST() { return d->CopyrightYear; }
  return getProperty(QCoreApplication::instance(),
                      "copyrightYear", "COPY_YEAR");
}

//-----------------------------------------------------------------------------
void vgApplication::setCopyrightYear(const QString& value)
{
  WITH_VGAPPLICATION_D()
    {
    d->CopyrightYear = value;
    return;
    }
  QCoreApplication::instance()->setProperty("copyrightYear", value);
}

//-----------------------------------------------------------------------------
void vgApplication::setCopyrightYear(int value)
{
  setCopyrightYear(QString::number(value));
}

//-----------------------------------------------------------------------------
QString vgApplication::copyrightOrganization()
{
  WITH_VGAPPLICATION_D_CONST() { return d->CopyrightOrganization; }
  const QString& fallback =
    getProperty(QCoreApplication::instance(),
                "copyrightOrganization", "COPY_ORGANIZATION");
  return (fallback.isEmpty() ? qApp->organizationName() : fallback);
}

//-----------------------------------------------------------------------------
void vgApplication::setCopyrightOrganization(const QString& value)
{
  WITH_VGAPPLICATION_D()
    {
    d->CopyrightOrganization = value;
    return;
    }
  QCoreApplication::instance()->setProperty("copyrightOrganization", value);
}

//-----------------------------------------------------------------------------
void vgApplication::setCopyright(const QString& year,
                                 const QString& organization)
{
  WITH_VGAPPLICATION_D()
    {
    d->CopyrightYear = year;
    d->CopyrightOrganization = organization;
    return;
    }
  QCoreApplication* const app = QCoreApplication::instance();
  app->setProperty("copyrightYear", year);
  app->setProperty("copyrightOrganization", organization);
}

//-----------------------------------------------------------------------------
void vgApplication::setCopyright(int year, const QString& organization)
{
  setCopyright(QString::number(year), organization);
}

//-----------------------------------------------------------------------------
QString vgApplication::userManualLocation()
{
  WITH_VGAPPLICATION_D_CONST() { return d->UserManualLocation; }

  const QCoreApplication* const app = QCoreApplication::instance();
  return app->property("userManualLocation").toString();
}

//-----------------------------------------------------------------------------
void vgApplication::setUserManualLocation(const QString& path)
{
  const QString& fullPath = resolveUserManualLocation(path);

  WITH_VGAPPLICATION_D()
    {
    d->UserManualLocation = fullPath;
    return;
    }
  QCoreApplication::instance()->setProperty("userManualLocation", fullPath);
}

//-----------------------------------------------------------------------------
void vgApplication::setupHelpMenu(
  QMenu* menu, vgApplication::HelpMenuEntries entries)
{
  if (entries.testFlag(vgApplication::UserManualAction))
    {
    menu->addAction(new vgUserManualAction(menu));
    }
  if (entries.testFlag(vgApplication::AboutAction))
    {
    menu->addAction(new vgAboutAction(menu));
    }
}

//-----------------------------------------------------------------------------
void vgApplication::addCommandLineOptions(qtCliArgs& args)
{
  qtCliOptions options;
  options.add("theme <file>", "Load application theme from 'file'");
  args.addOptions(options);
}

//-----------------------------------------------------------------------------
void vgApplication::parseCommandLine(qtCliArgs& args)
{
  const auto& themeFile = args.value("theme");
  if (!themeFile.isEmpty())
    {
    QSettings settings(themeFile, QSettings::IniFormat);

    QApplication::setStyle(settings.value("WidgetStyle").toString());
    QApplication::setPalette(qtColorScheme::fromSettings(settings));
    }
}
