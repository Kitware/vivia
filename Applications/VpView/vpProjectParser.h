/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpProjectParser_h
#define __vpProjectParser_h

// C++ includes.
#include <string>

#include <vtkSetGet.h>

// Forward declarations.
class vpProject;

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
  void SetFileName(const std::string& fileName)
    {
    this->ProjectFileName = fileName;
    }
  const std::string& GetProjectFileName() const
    {
    return this->ProjectFileName;
    }

  // Description:
  // Set/Get stream to use for configuration.
  void SetStream(const std::string& stream)
    {
    this->ProjectStream = stream;
    }

  // Description:
  // Parse the file now.
  bool Parse(vpProject* prj);

private:
  void ConstructCompletePath(std::string& fileName, const std::string& stem);

  int  CheckIfFileExists(const std::string& tag, const std::string& fileName);

  bool UseStream;

  std::string        ProjectFileName;
  std::string        ProjectStream;
};

#endif // __vpProjectParser_h
