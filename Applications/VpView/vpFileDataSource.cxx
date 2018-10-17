/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileDataSource.h"

#include <vgKwaUtil.h>

#include <qtNaturalSort.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QTextStream>

#include <algorithm>

namespace // anonymous
{

//-----------------------------------------------------------------------------
QByteArray readLine(QFile& f)
{
  auto result = f.readLine();

  const auto l = result.length();
  if (l > 0 && result[l-1] == '\n')
  {
    result = result.left(l - 1);
  }

  return result;
}

//-----------------------------------------------------------------------------
QStringList glob(const QDir& base, const QString& pattern)
{
  QStringList matches;

  // TODO handle multi-level globs
  foreach (const auto& p, base.entryList(QDir::Files))
  {
    if (QDir::match(pattern, p))
    {
      matches.append(base.filePath(p));
    }
  }

  return matches;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vpFileDataSourcePrivate
{
public:
  void readImageList(QFile& file, const QDir& baseDir);
  void readKwaIndex(QFile& file);

  QString DataSetSpecifier;
  QStringList DataFiles;

  bool MonitoringEnabled = false;
  QScopedPointer<QFileSystemWatcher> DataFilesWatcher;
};

QTE_IMPLEMENT_D_FUNC(vpFileDataSource)

//-----------------------------------------------------------------------------
void vpFileDataSourcePrivate::readImageList(QFile& file, const QDir& baseDir)
{
  while (!file.atEnd())
  {
    const auto line = readLine(file);
    if (line.isEmpty())
    {
      continue;
    }

    const auto path = baseDir.absoluteFilePath(QString::fromUtf8(line));
    if (!QFileInfo{path}.exists())
    {
      qWarning() << "Image data file" << path << "does not exist";
      continue;
    }

    qDebug() << "Archiving" << path;
    this->DataFiles.append(path);
  }
}

//-----------------------------------------------------------------------------
void vpFileDataSourcePrivate::readKwaIndex(QFile& file)
{
  const auto& versionLine = readLine(file);

  // Read index version
  bool okay = true;
  const auto version = versionLine.toInt(&okay);
  if (!okay)
  {
    qWarning().nospace()
      << "Unable to read data source " << this->DataSetSpecifier
      << ": " << versionLine << " is not a valid KWA version number";
    return;
  }
  if (version < 1 || version > 4)
  {
    qWarning().nospace()
      << "Unable to read data source " << this->DataSetSpecifier
      << ": KWA version " << version << " is not a supported";
    return;
  }

  // Read header
  auto dataName = QString::fromUtf8(readLine(file));
  if (!vgKwaUtil::resolvePath(dataName, this->DataSetSpecifier, "data"))
  {
    return;
  }
  if (version > 1)
  {
    if (version > 2)
    {
      // Read name of .meta file
      file.readLine();
    }

    // Read mission ID
    file.readLine();

    if (version > 3)
    {
      // Read stream ID
      file.readLine();
    }
  }

  // Read frames
  while (!file.atEnd())
  {
    // Read index line
    qint64 time;
    quint64 offset;

    auto line = QString::fromLatin1(readLine(file));
    QTextStream lineStream{&line};
    lineStream >> time >> offset;

    if (lineStream.status() != QTextStream::Ok)
    {
      qWarning() << "Failed to parse KWA index entry" << line;
      continue;
    }

    qDebug() << "Archiving" << dataName << offset;
    this->DataFiles.append(dataName + "@" + QString::number(offset));
  }
}

//-----------------------------------------------------------------------------
vpFileDataSource::vpFileDataSource() : d_ptr{new vpFileDataSourcePrivate}
{
}

//-----------------------------------------------------------------------------
vpFileDataSource::~vpFileDataSource()
{
}

//-----------------------------------------------------------------------------
void vpFileDataSource::setDataSetSpecifier(const QString& source)
{
  QTE_D();

  d->DataSetSpecifier = source;
  d->DataFilesWatcher.reset();
  this->update();
}

//-----------------------------------------------------------------------------
QString vpFileDataSource::dataSetSpecifier() const
{
  QTE_D();
  return d->DataSetSpecifier;
}

//-----------------------------------------------------------------------------
void vpFileDataSource::setDataFiles(const QStringList& filenames)
{
  QTE_D();

  d->DataSetSpecifier.clear();
  d->DataFiles = filenames;

  d->MonitoringEnabled = false;
  d->DataFilesWatcher.reset();
}

//-----------------------------------------------------------------------------
int vpFileDataSource::frames() const
{
  QTE_D();
  return d->DataFiles.count();
}

//-----------------------------------------------------------------------------
QString vpFileDataSource::frameName(int index) const
{
  QTE_D();
  return d->DataFiles.value(index);
}

//-----------------------------------------------------------------------------
QStringList vpFileDataSource::frameNames() const
{
  QTE_D();
  return d->DataFiles;
}

//-----------------------------------------------------------------------------
void vpFileDataSource::setMonitoringEnabled(bool enable)
{
  QTE_D();

  d->MonitoringEnabled = enable;
  if (enable)
  {
    this->update();
  }
  else
  {
    d->DataFilesWatcher.reset();
  }
}

//-----------------------------------------------------------------------------
void vpFileDataSource::update()
{
  QTE_D();

  if (d->DataSetSpecifier.isEmpty())
  {
    return;
  }

  d->DataFiles.clear();

  QString watchPath;
  QFileInfo fi{d->DataSetSpecifier};

  if (fi.exists())
  {
    watchPath = fi.absoluteFilePath();

    const auto baseDir = fi.absoluteDir();

    QFile file{d->DataSetSpecifier};
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      if (d->DataSetSpecifier.toLower().endsWith(".index"))
      {
        d->readKwaIndex(file);
      }
      else
      {
        d->readImageList(file, baseDir);
      }
    }
    else
    {
      qWarning().nospace()
        << "Unable to open data source " << d->DataSetSpecifier
        << ": " << file.errorString();
      return;
    }
  }
  else
  {
    const auto& dir = fi.absoluteDir();
    const auto& pattern = fi.fileName();

    // auto files = glob(QDir::current(), d->DataSetSpecifier); TODO
    auto files = glob(dir, pattern);
    std::sort(files.begin(), files.end(), qtNaturalSort::compare{});

    d->DataFiles = files;
    watchPath = dir.absolutePath();
    // TODO get watch paths for multi-level glob
  }

  // Begin monitoring the parent directory or the text file containing the
  // image filenames for changes. When a change is detected, we will update
  // ourselves automatically.
  if (d->MonitoringEnabled && !d->DataFilesWatcher && !watchPath.isEmpty())
  {
    // TODO handle multiple watch paths and the potential need to add paths
    // if new directories match a multi-level glob
    qDebug() << "Starting image data monitoring for path" << watchPath;
    d->DataFilesWatcher.reset(new QFileSystemWatcher);

    // Force watcher to use polling since the normal engine doesn't work
    // with network shares. See https://bugreports.qt.io/browse/QTBUG-8351.
    d->DataFilesWatcher->setObjectName(
      QLatin1String("_qt_autotest_force_engine_poller"));

    connect(d->DataFilesWatcher.data(), SIGNAL(directoryChanged(QString)),
            this, SLOT(updateDataFiles()));

    connect(d->DataFilesWatcher.data(), SIGNAL(fileChanged(QString)),
            this, SLOT(updateDataFiles()));

    d->DataFilesWatcher->addPath(watchPath);
    qDebug() << "Image data monitoring started";
  }
}

//-----------------------------------------------------------------------------
void vpFileDataSource::updateDataFiles()
{
  QTE_D();

  auto oldDataFiles = d->DataFiles;
  this->update();

  if (d->DataFiles != oldDataFiles)
  {
    emit this->frameSetChanged();
  }
}
