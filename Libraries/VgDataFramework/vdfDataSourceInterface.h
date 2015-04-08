/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vdfDataSourceInterface_h
#define __vdfDataSourceInterface_h

/// Base class for data source interfaces.
///
/// This class is used as a secondary base class for data source interfaces in
/// order to provide type checking for vdfDataSource::addInterface().
class vdfDataSourceInterface
{
public:
  virtual ~vdfDataSourceInterface() {}
};

#endif
