// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpFileDataSource.h"

#include "vpFileUtil.h"

#include <qtNaturalSort.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QStringList>

#include <algorithm>
#include <iostream>

#ifdef VISGUI_USE_GDAL
  #include <gdal_priv.h>
  #include <cpl_conv.h>
#endif

//-----------------------------------------------------------------------------
class vpFileDataSourcePrivate
{
public:
  QString DataSetSpecifier;
  QStringList DataFiles;

  bool MonitoringEnabled = false;
  QScopedPointer<QFileSystemWatcher> DataFilesWatcher;
};

QTE_IMPLEMENT_D_FUNC(vpFileDataSource)

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

bool add_mie4nitf_subdatasets (const QString &path, QStringList &list) {
#ifdef VISGUI_USE_GDAL
    GDALAllRegister();

  GDALDataset *dataset = \
        static_cast<GDALDataset *>(GDALOpen(path.toStdString().c_str(),
              GA_ReadOnly));

    if (!dataset)
    {
      qWarning().nospace()<< "GDAL could not load file." <<
        path.toStdString().c_str();
    }

    const char* subdatasets_domain_name = "SUBDATASETS";
    char **str = GDALGetMetadata(dataset, subdatasets_domain_name);

    int num_key_val_pairs = 0;
    while((*str) != NULL) {
      char *key_c_str= nullptr;
      QString key, value;
      const char *val_c_str = CPLParseNameValue((*str), &key_c_str);
      if (key_c_str != nullptr && val_c_str != nullptr)
      {
        key = QString(key_c_str);
        value = QString(val_c_str);
	
	// Subdataset # 2 inside `/A.ntf` would be indicated by:
        // SUBDATASET_2_NAME=NITF_IM:1:/A.ntf
        // SUBDATASET_2_DESC=Image 2 of A.ntf

        const QRegExp rgx = QRegExp("SUBDATASET_\\d+_NAME");

        if(key.contains(rgx))
          list.append(value);
      }
      CPLFree(key_c_str);
      ++(str);
      ++num_key_val_pairs;
    }
    assert(num_key_val_pairs % 2 == 0);
    assert(num_key_val_pairs / 2 == list.size());
    GDALClose(dataset);
    return true;
#else
    qWarning() << "ERROR: GDAL reader not found to open file: " <<
      path.toStdString();
     return false;
#endif

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
      while (!file.atEnd())
      {
        QString line = file.readLine().trimmed();
        if (line.isEmpty())
        {
          continue;
        }

        // The path we'd check existence of.  It may so happen, like in the case
        // of MIE4NITF, that the `path` to a frame doesn't exist on the disk.
        // For ex., in MIE4NITF, frame # 3 inside `/A.ntf` (0-indexed) is
        // stored in:
        // `NITF_IM:2:/A.ntf`.
        QString path_to_check;
        QStringList all_paths;

        // MIE4NITF are a bunch of frames inside an `NITF` file.  Just like we
        // use glob to get more than one frames, we use the prefix `MIE4NITF:`
        // to indicate that this file contains more than one frame in NITF
        // format.
        const QString mie4nitf_prefix = "MIE4NITF:";

        if (line.startsWith(mie4nitf_prefix))
        {
          int L = mie4nitf_prefix.length();
          QString p = line.mid(L);
          path_to_check = baseDir.absoluteFilePath(p);

          if(!add_mie4nitf_subdatasets(path_to_check, all_paths))
          {
            continue;
          }
        }
        else
        {
          path_to_check = baseDir.absoluteFilePath(line);
          all_paths.append(path_to_check);
        }

        if (!QFileInfo{path_to_check}.exists())
        {
          qWarning() << "Image data file" << path_to_check << "does not exist";
          continue;
        }

        for(auto path: all_paths)
        {
          qDebug() << "Archiving" << path;
          d->DataFiles.append(path);
        }
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
    auto files = vpGlobFiles(dir, pattern);
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
