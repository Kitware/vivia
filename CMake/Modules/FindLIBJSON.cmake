#ckwg +4
# Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

# Locate the system installed PROJ
#
# The following variables will guide the build:
#
# LIBJSON_ROOT        - Set to the install prefix of the PROJ library
# LIBJSON_LIBNAME     - Name of the installed library (defaults to json)
#
# The following variables will be set:
#
# LIBJSON_FOUND       - Set to true if PROJ can be found
# LIBJSON_INCLUDE_DIR - The path to the PROJ header files
# LIBJSON_LIBRARY     - The full path to the PROJ library

if( LIBJSON_DIR )
  find_package( LIBJSON NO_MODULE )
elseif( NOT LIBJSON_FOUND )

  if(NOT LIBJSON_LIBNAME)
    set(LIBJSON_LIBNAME json)
  endif()

  # Backup the previous root path
  if(LIBJSON_ROOT)
    set(_LIBJSON_ROOT_OPTS_INCLUDE ${LIBJSON_ROOT}/include/${LIBJSON_LIBNAME})
    set(_LIBJSON_ROOT_OPTS_LIB     ${LIBJSON_ROOT}/lib)
  endif()

  find_path(LIBJSON_INCLUDE_DIR ${LIBJSON_LIBNAME}.h ${_LIBJSON_ROOT_OPTS_INCLUDE})
  find_library(LIBJSON_LIBRARY ${LIBJSON_LIBNAME} ${_LIBJSON_ROOT_OPTS_LIB})

  include( FindPackageHandleStandardArgs )
  FIND_PACKAGE_HANDLE_STANDARD_ARGS( LIBJSON LIBJSON_INCLUDE_DIR LIBJSON_LIBRARY )
endif()
