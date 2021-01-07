// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QScopedPointer>
#include <QSettings>
#include <QStringList>

#include <qtCliArgs.h>
#include <qtCliOptions.h>
#include <qtMap.h>

#define CHECK_FILE_ARG(_n) \
  if (!checkFileArg(_n, args)) return 1

#define qOut() QTextStream(stdout, QIODevice::WriteOnly)

typedef int (*Command)(QSettings&, const QStringList&);

static qtCliArgs* gArgs = 0;

//-----------------------------------------------------------------------------
class QDebugWithNewline : public QDebug
{
public:
  inline QDebugWithNewline(const QDebug& other) : QDebug(other) {}
  inline ~QDebugWithNewline() { this->nospace() << '\n'; }
};

//-----------------------------------------------------------------------------
static QDebugWithNewline error()
{
  static QFile err;
  err.isOpen() || err.open(stderr, QIODevice::WriteOnly);

  static const QString prefix = QString("%1:").arg(gArgs->executableName());
  return QDebugWithNewline(QDebug(&err) << qPrintable(prefix));
}

//-----------------------------------------------------------------------------
static QString joinSorted(QStringList list, const QString& sep)
{
  qSort(list);
  return list.join(sep);
}

//-----------------------------------------------------------------------------
void copyValue(const QString& key, QSettings* dst, const QSettings* src)
{
  dst->setValue(key, src->value(key));
}

//-----------------------------------------------------------------------------
void ignoreArgs(const char* command, const QStringList& args, int required)
{
  const int given = args.count();
  if (given > required)
    {
    const QString prefix = QString("%1: warning:").arg(command);
    const int extra = given - required;
    qWarning() << qPrintable(prefix) << "was passed" << given
               << "arguments, but only" << required << "required";
    qWarning() << qPrintable(prefix) << "ignoring" << extra
               << (extra > 1 ? "extra arguments" : "extra argument");
    }
}

//-----------------------------------------------------------------------------
bool checkFileArg(const char* command, const QStringList& args)
{
  ignoreArgs(command, args, 1);

  QFileInfo fi(args.first());
  if (!fi.exists())
    {
    qWarning() << qPrintable(QString("%1: error:").arg(command))
               << "input file" << args.first() << "not found";
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
static void showValue(QSettings& store, const QString& key, QString prefix)
{
  if (!prefix.isEmpty())
    {
    prefix = prefix.mid(1) + key + '=';
    }

  QVariant data = store.value(key);
  if (!data.isNull())
    {
    if (data.canConvert(QVariant::String))
      {
      qOut() << prefix << data.toString() << '\n';
      }
    else
      {
      qOut() << prefix << "<value cannot be represented textually>\n";
      }
    }
}

//-----------------------------------------------------------------------------
static void showGroup(QSettings& store, const QString& key, QString prefix)
{
  if (!key.isEmpty())
    {
    store.beginGroup(key);
    prefix += key + '/';
    }

  foreach (const QString& ckey, store.childKeys())
    showValue(store, ckey, prefix);
  foreach (const QString& group, store.childGroups())
    showGroup(store, group, prefix);

  if (!key.isEmpty())
    {
    store.endGroup();
    }
}

//-----------------------------------------------------------------------------
static void showKey(
  QSettings& store, const QString& key, const QString& prefix)
{
  if (key.isEmpty() || store.childGroups().contains(key))
    {
    showGroup(store, key, prefix + '/');
    }
  else
    {
    showValue(store, key, QString());
    }
}

//-----------------------------------------------------------------------------
static int commandImport(QSettings& store, const QStringList& args)
{
  CHECK_FILE_ARG("import");

  QSettings overlay(args.first(), QSettings::IniFormat);
  qtUtil::map(overlay.allKeys(), &copyValue, &store, &overlay);
  return 0;
}

//-----------------------------------------------------------------------------
static int commandReplace(QSettings& store, const QStringList& args)
{
  CHECK_FILE_ARG("replace");

  QSettings overlay(args.first(), QSettings::IniFormat);
  store.clear();
  qtUtil::map(overlay.allKeys(), &copyValue, &store, &overlay);
  return 0;
}

//-----------------------------------------------------------------------------
static int commandSet(QSettings& store, const QStringList& args)
{
  if (args.count() < 2)
    {
    error() << "set: insufficient arguments";
    error() << "Usage: set <key> <value>";
    return 1;
    }
  ignoreArgs("set", args, 2);

  store.setValue(args[0], args[1]);
  return 0;
}

//-----------------------------------------------------------------------------
static int commandUnset(QSettings& store, const QStringList& args)
{
  if (args.count() < 1)
    {
    error() << "unset: insufficient arguments";
    error() << "Usage: unset <key>";
    return 1;
    }
  ignoreArgs("unset", args, 1);

  store.remove(args[0]);
  return 0;
}

//-----------------------------------------------------------------------------
static int commandShow(QSettings& store, const QStringList& args)
{
  ignoreArgs("show", args, 1);

  // Enter specified group
  QString key = args.value(0, QString()), prefix;
  int gs;
  while ((gs = key.indexOf('/')) >= 0)
    {
    if (gs + 1 < key.length())
      {
      prefix += '/' + key.left(gs);
      }
    key = key.mid(gs + 1);
    }

  // Show requested key or group
  if (!prefix.isEmpty())
    {
    store.beginGroup(prefix.mid(1));
    showKey(store, key, prefix);
    store.endGroup();
    }
  else
    {
    showKey(store, key, QString());
    }
  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // Set up About information
  QCoreApplication::setApplicationName("QSettings Tool");
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");
  QCoreApplication::setApplicationVersion("1.0");

  // Register commands
  QHash<QString, Command> commands;
  commands.insert("import",    &commandImport);
  commands.insert("replace",   &commandReplace);
  commands.insert("set",       &commandSet);
  commands.insert("unset",     &commandUnset);
  commands.insert("show",      &commandShow);

  // Set up command line options
  qtCliArgs args(argc, argv);
  qtCliOptions options;
  options.add("o").add("organization <name>",
                       "Organization name of the settings to manipulate",
                       qtCliOption::Required);
  options.add("a").add("application <name>",
                       "Application name of the settings to manipulate");
  options.add("s").add("system", "Operate in the System scope,"
                       " rather than User scope");

  const QString joinedCommandNames = joinSorted(commands.keys(), ", ");

  qtCliOptions namedArgs;
  namedArgs.add("command", "Command to execute; one of " + joinedCommandNames,
                qtCliOption::Required);
  namedArgs.add("<args>", "Additional arguments to the command");

  args.addOptions(options);
  args.addNamedArguments(namedArgs);

  // Parse arguments
  args.parseOrDie();
  gArgs = &args;

  // Create the application instance
  QCoreApplication app(args.qtArgc(), args.qtArgv());

  const QString cname = args.value("command");
  Command command = commands.value(cname, 0);
  if (command)
    {
    QSettings store(args.isSet("system") ? QSettings::SystemScope
                                         : QSettings::UserScope,
                    args.value("organization"),
                    args.value("application"));
    return (*command)(store, args.args().mid(1));
    }
  else
    {
    error() << "error: command" << cname << "not known";
    return 1;
    }
}
