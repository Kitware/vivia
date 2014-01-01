/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgApplication.h"

#include "vgAboutAction.h"

#include <QMenu>
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

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vgApplicationPrivate
{
public:
  QString CopyrightYear;
  QString CopyrightOrganization;
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
void vgApplication::setupHelpMenu(
  QMenu* menu, vgApplication::HelpMenuEntries entries)
{
  if (entries.testFlag(vgApplication::AboutAction))
    {
    menu->addAction(new vgAboutAction(menu));
    }
  // TODO 'user manual' action
}
