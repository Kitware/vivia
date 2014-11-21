/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpFileDataSource.h"

#include <vtksys/Glob.hxx>
#include <vtksys/SystemTools.hxx>

// C++ includes.
#include <iostream>
#include <fstream>

// VTK includes.
#include <vtkNew.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>

// Qt includes.
#include <QFileSystemWatcher>
#include <QMutexLocker>
#include <QStringList>

#include <qtStlUtil.h>

//-----------------------------------------------------------------------------
vpFileDataSource::vpFileDataSource() :
  MonitoringEnabled(false), DataFilesMutex(QMutex::Recursive)
{
  this->DataFiles = vtkSmartPointer<vtkStringArray>::New();
}

//-----------------------------------------------------------------------------
vpFileDataSource::~vpFileDataSource()
{
}

//-----------------------------------------------------------------------------
void vpFileDataSource::setDataSetSpecifier(const std::string& source)
{
  this->DataSetSpecifier = source;
  this->ModifiedTime.Modified();
}

//-----------------------------------------------------------------------------
std::string vpFileDataSource::getDataSetSpecifier()
{
  return this->DataSetSpecifier;
}

//-----------------------------------------------------------------------------
void vpFileDataSource::setDataFiles(const std::vector<std::string>& filenames)
{
  this->clear();
  this->DataSetSpecifier.clear();

  for (size_t i = 0, size = filenames.size(); i < size; ++i)
    {
    this->DataFiles->InsertNextValue(filenames[i]);
    }
}

//-----------------------------------------------------------------------------
int vpFileDataSource::getFileCount()
{
  QMutexLocker ml(&this->DataFilesMutex);
  this->update();
  return static_cast<int>(this->DataFiles->GetNumberOfValues());
}

//-----------------------------------------------------------------------------
std::string vpFileDataSource::getDataFile(int index)
{
  QMutexLocker ml(&this->DataFilesMutex);
  this->update();

  if (index < 0 || index >= this->DataFiles->GetNumberOfValues())
    {
    return std::string();
    }

  return this->DataFiles->GetValue(index);
}

//-----------------------------------------------------------------------------
void vpFileDataSource::setMonitoringEnabled(bool enable)
{
  QMutexLocker ml(&this->DataFilesMutex);
  this->ModifiedTime.Modified();
  this->MonitoringEnabled = enable;
  if (!enable)
    {
    this->DataFilesWatcher.reset();
    }
  else
    {
    this->update();
    }
}

//-----------------------------------------------------------------------------
void vpFileDataSource::update()
{
  QMutexLocker ml(&this->DataFilesMutex);
  if (this->UpdateTime > this->ModifiedTime)
    {
    return;
    }

  this->UpdateTime.Modified();

  if (this->DataSetSpecifier.empty())
    {
    return;
    }

  // First clear any data structure before we read new data source.
  this->clear();

  std::string line;
  std::string watchPath;

  if (vtksys::SystemTools::FileExists(this->DataSetSpecifier.c_str(), true))
    {
    std::string basePath =
      vtksys::SystemTools::GetFilenamePath(this->DataSetSpecifier.c_str());
    basePath.append("/");

    std::string completePath;
    watchPath = this->DataSetSpecifier;

    std::ifstream file(this->DataSetSpecifier.c_str());

    if (file.is_open())
      {
      while (!file.eof() || file.good())
        {
        std::getline(file, line);

        if (line.empty())
          {
          continue;
          }

        // If each line is not a full path then append the base path or
        // if the file is not found try finding the file using base path.
        if (!vtksys::SystemTools::FileIsFullPath(line.c_str()) ||
            !vtksys::SystemTools::FileExists(line.c_str(), true))
          {
          completePath = basePath + line;
          }
        else
          {
          completePath = line;
          }

        if (!vtksys::SystemTools::FileExists(completePath.c_str(), true))
          {
          std::cerr << "Image data file (" << completePath
                    << ") does not exist. " << std::endl;
          continue;
          }

        std::cerr << "Archiving (" << completePath << "). " << std::endl;

        this->DataFiles->InsertNextValue(completePath);
        }
      }
    else
      {
      std::cerr << "Unable to open this  (" << this->DataSetSpecifier
                << ") data source. " << std::endl;
      return;
      }
    }
  else
    {
    // Interpret the specifier as a glob
    vtksys::Glob glob;
    glob.FindFiles(this->DataSetSpecifier);
    glob.SetRecurse(true);
    std::vector<std::string>& files = glob.GetFiles();
    if (files.empty())
      {
      return;
      }

    int numFiles = static_cast<int>(files.size());
    vtkNew<vtkStringArray> fileNames;
    fileNames->SetNumberOfValues(numFiles);
    for (int i = 0; i < numFiles; i++)
      {
      fileNames->SetValue(i, files[i].c_str());
      }

    vtkNew<vtkSortFileNames> sort;
    sort->NumericSortOn();
    sort->SetInputFileNames(fileNames.GetPointer());
    this->DataFiles = sort->GetFileNames();

    watchPath =
     vtksys::SystemTools::GetFilenamePath(this->DataFiles->GetValue(0));
    }

  // Begin monitoring the parent directory or the text file containing the
  // image filenames for changes. When a change is detected, we will update
  // ourselves automatically.
  if (this->MonitoringEnabled &&
      !this->DataFilesWatcher && !watchPath.empty())
    {
    std::cout << "Starting image data monitoring.\n";
    this->DataFilesWatcher.reset(new QFileSystemWatcher);

    // Force watcher to use polling since the normal engine doesn't work
    // with network shares.
    this->DataFilesWatcher->setObjectName(
      QLatin1String("_qt_autotest_force_engine_poller"));

    connect(this->DataFilesWatcher.data(),
            SIGNAL(directoryChanged(QString)),
            this, SLOT(updateDataFiles()));

    connect(this->DataFilesWatcher.data(),
            SIGNAL(fileChanged(QString)),
            this, SLOT(updateDataFiles()));

    this->DataFilesWatcher->addPath(qtString(watchPath));
    std::cout << "Image data monitoring started.\n";
    }
}

//-----------------------------------------------------------------------------
void vpFileDataSource::clear()
{
  QMutexLocker ml(&this->DataFilesMutex);
  this->DataFiles->Reset();
}

//-----------------------------------------------------------------------------
void vpFileDataSource::updateDataFiles()
{
    {
    QMutexLocker ml(&this->DataFilesMutex);
    this->ModifiedTime.Modified();
    this->update();
    }
  emit this->dataFilesChanged();
}
