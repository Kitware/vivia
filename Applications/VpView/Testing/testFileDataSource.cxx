/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
