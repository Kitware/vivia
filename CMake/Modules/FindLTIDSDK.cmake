# Copyright 2012 by Kitware, Inc. All Rights Reserved.
#
# Author: Chuck Atkins <chuck dot atkins at kitware dot com>
#
# Locate the LizardTech Raster Decode SDK
#
# If set, the following variable will be used to guide the search
# LTIDSDK_ROOT        - The locations of the extracted Raster_DSDK folder
#
# The following variables will be set:
#
# LTIDSDK_FOUND       - Set to true if Decode SDK can be found
# LTIDSDK_INCLUDE_DIR - The path to the SDK  header files
# LTIDSDK_LIBRARIES   - The full path to the SDK libraries

if( NOT LTIDSDK_FOUND )

  if (WIN32)
    set(_lti_dsdk_lib lti_dsdk)
  else()
    set(_lti_dsdk_lib ltidsdk)
  endif()

  if( LTIDSDK_ROOT )
    set( LTIDSDK_INCLUDE_PATH_HINT ${LTIDSDK_ROOT}/include )
    set( LTIDSDK_LIBRARY_PATH_HINT ${LTIDSDK_ROOT}/lib )
  else( LTIDSDK_ROOT )
    set( LTIDSDK_INCLUDE_PATH_HINT )
    set( LTIDSDK_LIBRARY_PATH_HINT )
  endif( LTIDSDK_ROOT )

  find_path( LTIDSDK_INCLUDE_DIR lt_base.h ${LTIDSDK_INCLUDE_PATH_HINT} )
  find_library( LTIDSDK_LIBRARIES ${_lti_dsdk_lib} ${LTIDSDK_LIBRARY_PATH_HINT} )
  if(LTIDSDK_LIBRARIES)
    find_package(Threads REQUIRED)
    list(APPEND LTIDSDK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  endif()

  # Determine the SDK version
  if(LTIDSDK_INCLUDE_DIR)
    file( READ ${LTIDSDK_INCLUDE_DIR}/lti_version.h LTIDSDK_VERSION_FILE )
    string( REGEX MATCH "LTI_SDK_MAJOR *[0-9]+"
      LTIDSDK_VERSION_MAJOR "${LTIDSDK_VERSION_FILE}" )
    string( REGEX REPLACE "LTI_SDK_MAJOR *" ""
      LTIDSDK_VERSION_MAJOR "${LTIDSDK_VERSION_MAJOR}" )
    string( REGEX MATCH "LTI_SDK_MINOR *[0-9]+"
      LTIDSDK_VERSION_MINOR "${LTIDSDK_VERSION_FILE}" )
    string( REGEX REPLACE "LTI_SDK_MINOR *" ""
      LTIDSDK_VERSION_MINOR "${LTIDSDK_VERSION_MINOR}" )
    string( REGEX MATCH "LTI_SDK_REV *[0-9]+"
      LTIDSDK_VERSION_PATCH "${LTIDSDK_VERSION_FILE}" )
    string( REGEX REPLACE "LTI_SDK_REV *" ""
      LTIDSDK_VERSION_PATCH "${LTIDSDK_VERSION_PATCH}" )
    set(LTIDSDK_VERSION "${LTIDSDK_VERSION_MAJOR}.${LTIDSDK_VERSION_MINOR}.${LTIDSDK_VERSION_PATCH}")
  else()
    set(LTIDSDK_VERSION "0.0.0")
  endif()

  include( FindPackageHandleStandardArgs )
  FIND_PACKAGE_HANDLE_STANDARD_ARGS( LTIDSDK
    REQUIRED_VARS LTIDSDK_INCLUDE_DIR LTIDSDK_LIBRARIES
    VERSION_VAR LTIDSDK_VERSION
  )
endif(NOT LTIDSDK_FOUND)
