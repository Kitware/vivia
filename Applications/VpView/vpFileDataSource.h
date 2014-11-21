/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpFileDataSource_h
#define __vpFileDataSource_h

// C++ STL includes.
#include <string>
#include <vector>

#include <QMutex>
#include <QObject>
#include <QScopedPointer>

#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

// Forward declarations.
class QFileSystemWatcher;

class vtkStringArray;

class vpFileDataSource : public QObject
{
  Q_OBJECT

public:
  // Description:
  // Constructor / destructor
  vpFileDataSource();
  ~vpFileDataSource();

  // Description:
  // Set the data source. Currently it takes the filename
  // or the Glob.
  void setDataSetSpecifier(const std::string& source);
  std::string getDataSetSpecifier();

  // Description:
  // Set the data set directly from an ordered vector of filenames
  void setDataFiles(const std::vector<std::string>& filenames);

  int getFileCount();

  // Description:
  // Return a specific file using the index [0, size - 1]
  std::string getDataFile(int index);

  // Description:
  // Enable monitoring for data set changes
  void setMonitoringEnabled(bool enable);

signals:
  void dataFilesChanged();

protected:
  void update();
  void clear();

protected slots:
  void updateDataFiles();

private:
  std::string DataSetSpecifier;
  vtkSmartPointer<vtkStringArray> DataFiles;

  vtkTimeStamp ModifiedTime;
  vtkTimeStamp UpdateTime;

  bool MonitoringEnabled;
  QScopedPointer<QFileSystemWatcher> DataFilesWatcher;
  QMutex DataFilesMutex;
};

#endif // __vpFileDataSource_h
