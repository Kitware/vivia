/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpProjectParser_h
#define __vpProjectParser_h

#include "vpProject.h"

#include <qtStlUtil.h>

#include <vtkSetGet.h>

#include <string>

class vpProjectParser
{
public:
  // Description:
  // Constructor / Destructor.
  vpProjectParser();
  ~vpProjectParser();

  // Description:
  // Use stream to read the project configuration (rather than file).
  // Default is set to false.
  void SetUseStream(bool flag)
    {
    this->UseStream = flag;
    }
  bool GetUseStream() const
    {
    return this->UseStream;
    }

  // Description:
  // Set/Get name of the configuration file that needs
  // to be parsed.
  void SetFileName(const QString& fileName)
    {
    this->ProjectFileName = fileName;
    }
  const QString& GetProjectFileName() const
    {
    return this->ProjectFileName;
    }

  // Description:
  // Set/Get stream to use for configuration.
  void SetStream(const QByteArray& stream)
    {
    this->ProjectStream = stream;
    }
  void SetStream(const std::string& stream)
    {
    this->ProjectStream = qtBytes(stream);
    }

  // Description:
  // Parse the file now.
  bool Parse(vpProject* prj);

private:
  static vpProject::FileState CheckIfFileExists(
    const QString& tag, const QString& fileName);

  bool UseStream;

  QString    ProjectFileName;
  QByteArray ProjectStream;
};

#endif // __vpProjectParser_h
