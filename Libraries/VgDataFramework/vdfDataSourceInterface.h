// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
