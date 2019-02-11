/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <cstdio>

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTextStream>

QTextStream* qerr;

enum
{
  ERR_NONE = 0,
  ERR_BAD_ARGS,
  ERR_DOES_NOT_EXIST,
  ERR_IO
};

//-----------------------------------------------------------------------------
QString formatGuard(QString format, QString fileName)
{
  fileName.replace(QRegExp("[^A-Za-z0-9_]"), "_");

  // Find and replace placeholders
  QRegExp re("%[^%]*%");
  int n;
  while ((n = re.indexIn(format)) >= 0)
    {
    // Extract placeholder format
    bool uc = false, lc = false;
    QString f = re.cap(0);
    f = f.mid(1, f.length() - 2);
    if (f.startsWith('l', Qt::CaseInsensitive))
      {
      f = f.mid(1);
      lc = true;
      }
    else if (f.startsWith('u', Qt::CaseInsensitive))
      {
      f = f.mid(1);
      uc = true;
      }

    int start = 0;
    int length = -1;

    QString ns = f.left(f.indexOf(':'));
    if (!ns.isEmpty() || f.startsWith(':'))
      {
      start = ns.toInt();
      (start < 0) && (start = fileName.length() + start);
      ns = f.mid(ns.length() + 1);
      if (!ns.isEmpty())
        {
        length = ns.toInt();
        (length < 0) && (length = fileName.length() + length - start);
        }
      }

    // Replace placeholder
    QString rp = fileName.mid(start, length);
    lc && (rp = rp.toLower(), true);
    uc && (rp = rp.toUpper(), true);
    format.replace(n, re.matchedLength(), rp);
    }

  return format;
}

//-----------------------------------------------------------------------------
int fixHeaderGuards(const QString& format, const QString& fileName)
{
  QFile file(fileName);
  if (!file.exists())
    {
    *qerr << "Error: File \"" << fileName << "\" not found.\n";
    return ERR_DOES_NOT_EXIST;
    }
  if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
    *qerr << "Error: Unable to open file \"" << fileName << "\": "
          << file.errorString() << '\n';
    return ERR_IO;
    }

  QString guard = formatGuard(format, QFileInfo(file).fileName());

  bool c = false, d = false, e = false;
  QRegExp cre("^(\\s*#ifndef\\s+)([A-Za-z0-9_]+)(\\s+.*)?$");
  QRegExp dre("^(\\s*#define\\s+)([A-Za-z0-9_]+)(\\s+.*)?$");
  QRegExp ere;

  QStringList lines;
  while (!file.atEnd())
    {
    QString line = QString::fromUtf8(file.readLine());
    // Replace 'ifndef' of guard
    if (!c && cre.exactMatch(line))
      {
      line = cre.cap(1) + guard + cre.cap(3);
      ere = QRegExp("^(\\s*#endif\\s*//\\s*)" + cre.cap(2) + "(.*)$");
      e = true;
      c = true;
      }
    // Replace 'define' of guard
    if (!d && dre.exactMatch(line))
      {
      line = dre.cap(1) + guard + dre.cap(3);
      d = true;
      }
    // Replace 'endif' of guard
    if (e && ere.exactMatch(line))
      {
      line = ere.cap(1) + guard + ere.cap(3);
      }
    lines << line;
    }

  file.seek(0);
  file.resize(0);
  file.write(lines.join(QString()).toUtf8());

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  QCoreApplication app{argc, argv};

  // Set up QTextStream to stderr
  QFile ferr;
  ferr.open(stderr, QIODevice::WriteOnly | QIODevice::Text);
  QScopedPointer<QTextStream> qerrPtr(new QTextStream(&ferr));
  qerr = qerrPtr.data();

  // Check arguments
  QStringList files = app.arguments();
  if (files.count() < 3)
    {
    *qerr << "Usage: " << QFileInfo(files[0]).fileName()
          << " <format> <file> [<file>...]\n";
    return ERR_BAD_ARGS;
    }

  // Get format and list of files
  files.removeFirst();
  QString format = files.takeFirst();

  // Check format is valid
  if (!format.contains(QRegExp("%[^%]*%")))
    {
    *qerr << "Invalid format \"" << format << "\".\n"
          << "Format must contain at least one placeholder sequence.\n";
    return ERR_BAD_ARGS;
    }

  // Adjust files
  foreach (QString fname, files)
    {
    int result = fixHeaderGuards(format, fname);
    if (result)
      {
      return result;
      }
    }

  return 0;
}
