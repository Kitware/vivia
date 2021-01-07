// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpFileDataSource.h"

#include <vector>
#include <string>

int main(int argc, char** argv)
{
  int retVal = 0;

  if (argc < 3)
    {
    return VTK_ERROR;
    }

  vpFileDataSource* dataSource = new vpFileDataSource();
  dataSource->setDataSetSpecifier(argv[1]);

  if (dataSource->getFileCount() != 762)
    {
    retVal = VTK_ERROR;
    }

  dataSource->setDataSetSpecifier(argv[2]);

  if (dataSource->getFileCount() != 762)
    {
    retVal = VTK_ERROR;
    }

  delete dataSource;

  return retVal;
}
