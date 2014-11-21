#ckwg +4
# Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

# Locate the system installed PROJ
#
# The following variables will guide the build:
#
# GDAL_ROOT        - Set to the install prefix of the PROJ library
#
# The following variables will be set:
#
# GDAL_FOUND       - Set to true if PROJ can be found
# GDAL_INCLUDE_DIR - The path to the PROJ header files
# GDAL_LIBRARY     - The full path to the PROJ library

if( GDAL_DIR )
  find_package( GDAL NO_MODULE )
elseif( NOT GDAL_FOUND )
  if (WIN32)
    set(_gdal_library gdal_i)
  else()
    set(_gdal_library gdal)
  endif()

  # Backup the previous root path
  if(GDAL_ROOT)
    set(_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH ${GDAL_ROOT})
    set(_GDAL_ROOT_OPTS ONLY_CMAKE_FIND_ROOT_PATH)
  endif()

  find_path(GDAL_INCLUDE_DIR gdal.h ${_GDAL_ROOT_OPTS} PATH_SUFFIXES gdal)
  find_library( GDAL_LIBRARY ${_gdal_library} ${_GDAL_ROOT_OPTS})

  # Restore the original root path
  if(GDAL_ROOT)
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
  endif()

  include( FindPackageHandleStandardArgs )
  FIND_PACKAGE_HANDLE_STANDARD_ARGS( GDAL GDAL_INCLUDE_DIR GDAL_LIBRARY )
endif()
