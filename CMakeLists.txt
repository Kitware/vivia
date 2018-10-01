# Root cmake file.
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(visGUI)

# Include core CMake support code.
include(CMake/functions.cmake)

# Find dependencies. These manage to be more complicated than simple calls to
# find_package with remarkable consistency, so are in a separate file.
set(global_library_search_paths)
include(CMake/coredeps.cmake)

# We currently build static libraries by default. This might change in the
# future.
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
set(VISGUI_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
if(NOT BUILD_SHARED_LIBS)
  message(WARNING
    "Building static libraries has known problems related to Qt meta-objects. "
    "Any application using plugins will likely crash. "
    "This configuration is NOT recommended.")
endif()

# Set default visibility to hidden when building shared
if(BUILD_SHARED_LIBS)
  include(CheckCCompilerFlag)
  include(CheckCXXCompilerFlag)
  check_c_compiler_flag(-fvisibility=hidden C_HAS_VISIBILITY_FLAG)
  check_cxx_compiler_flag(-fvisibility=hidden CXX_HAS_VISIBILITY_FLAG)
  if(C_HAS_VISIBILITY_FLAG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  endif()
  if(CXX_HAS_VISIBILITY_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
  endif()
else()
  add_definitions(-DVISGUI_STATIC)
endif()

# Place executable and libraries in a common location, rather than their
# respective build directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(VTK_MODULES_DIR "${visGUI_BINARY_DIR}/${VTK_INSTALL_PACKAGE_DIR}/Modules")
set(VTK_INSTALL_PYTHON_USING_CMAKE TRUE)

# Create ABI exports header
include(CMake/exports.cmake)
vg_generate_export_header(
  "${PROJECT_BINARY_DIR}/vgExport.h"
  "${PROJECT_SOURCE_DIR}/vgExport.cmake"
)
install_headers("${PROJECT_BINARY_DIR}/vgExport.h" TARGET ${PROJECT_NAME})

# Allow subprojects to find configured headers
include_directories(${PROJECT_BINARY_DIR})

# Include subdirectories to build.
add_subdirectory(Libraries)
