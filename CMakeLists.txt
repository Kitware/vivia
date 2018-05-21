# Root cmake file.
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(visGUI)

include(vgVersion.cmake)

# Include essential cmake modules.
include(CMakeDependentOption)

# User-configurable build options for...
#   ...Applications
option(VISGUI_ENABLE_VIQUI          "Build viqui application"               ON)
option(VISGUI_ENABLE_VPVIEW         "Build vpView application"              ON)
option(VISGUI_ENABLE_VSPLAY         "Build vsPlay application"              ON)
#   ...External library features
option(VISGUI_ENABLE_GDAL           "Enable features that require GDAL"     OFF)
option(VISGUI_ENABLE_KWPPT          "Enable features that require KWPPT"    OFF)
option(VISGUI_ENABLE_VIDTK          "Enable features that require VidTK"    ON)
option(VISGUI_ENABLE_KWIVER         "Enable features that require KWIVER"   ON)
#   ...Python bindings
option(VISGUI_ENABLE_PYTHON         "Enable VisGUI Python bindings"         OFF)
#   ...Install options
option(VISGUI_DISABLE_FIXUP_BUNDLE  "Disable install fixup bundle utility"  OFF)
#   ...General features
option(BUILD_TESTING                "Build tests"                           OFF)
option(BUILD_SDK_DOCUMENTATION      "Build SDK documentation"               OFF)
option(BUILD_USER_DOCUMENTATION     "Build user documentation"              OFF)

# Include core CMake support code.
include(CMake/functions.cmake)
include(CMake/wrapping.cmake)
include(CMake/doxygen.cmake)

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

# Include testing support.
include(CMake/testing.cmake)

# Create ABI exports header
include(CMake/exports.cmake)
vg_generate_export_header(
  "${PROJECT_BINARY_DIR}/vgExport.h"
  "${PROJECT_SOURCE_DIR}/vgExport.cmake"
)
install_headers("${PROJECT_BINARY_DIR}/vgExport.h" TARGET ${PROJECT_NAME})

# Declare exports file
set(VisGUI_EXPORT_FILE "${visGUI_BINARY_DIR}/VisGUIConfigTargets.cmake")
if(EXISTS "${VisGUI_EXPORT_FILE}")
  file(REMOVE "${VisGUI_EXPORT_FILE}")
endif()

# Create package config for build directory
set(VisGUI_INCLUDE_DIR "${visGUI_BINARY_DIR}/include")
configure_file(
  CMake/VisGUIConfig.cmake.in "${visGUI_BINARY_DIR}/VisGUIConfig.cmake"
  @ONLY
)

# Create package config for install directory
set(VisGUI_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
configure_file(
  CMake/VisGUIConfig.cmake.in "${visGUI_BINARY_DIR}/VisGUIConfig-install.cmake"
  @ONLY
)

# Configure developer environment
configure_file(
  setupDevel.sh.in "${visGUI_BINARY_DIR}/setupDevel.sh" @ONLY
)

install(FILES "${visGUI_BINARY_DIR}/VisGUIConfig-install.cmake"
        COMPONENT Development
        DESTINATION lib/cmake RENAME VisGUIConfig.cmake
)
install(EXPORT VisGUI
        COMPONENT Development
        DESTINATION lib/cmake FILE VisGUIConfigTargets.cmake
)

# Allow subprojects to find configured headers
include_directories(${PROJECT_BINARY_DIR})

if(VISGUI_ENABLE_PYTHON)
  find_package(PythonInterp REQUIRED VERSION 2.7)
  find_package(PythonLibs REQUIRED)
  find_package(Shiboken REQUIRED)
  find_package(PySide REQUIRED)

  set(PYTHON_SHORT python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})

  # Create 'virtual modules' for use as wrapping dependencies, starting with
  # common properties (note that any PySide dependency requires using the
  # pyside_global.h header, which in turn brings in all of Qt that is wrapped
  # by PySide, hence every 'virtual module' needs include paths for everything)
  foreach(_module Core Gui)
    set(PySide:${_module}_TYPESYSTEM_PATHS ${PYSIDE_TYPESYSTEMS})
    set(PySide:${_module}_WRAP_INCLUDE_DIRS
      ${PYSIDE_INCLUDE_DIR}
      ${PYSIDE_INCLUDE_DIR}/QtCore
      ${PYSIDE_INCLUDE_DIR}/QtGui
    )
    set(PySide:${_module}_WRAP_LINK_LIBRARIES ${PYSIDE_LIBRARY})
    set(PySide:${_module}_GLOBAL_HEADER pyside_global.h)
  endforeach()

  # Set typesystem for each 'virtual module'
  set(PySide:Core_TYPESYSTEM "typesystem_core.xml")
  set(PySide:Gui_TYPESYSTEM "typesystem_gui.xml")
endif()

# Include subdirectories to build.
add_subdirectory(Libraries)
add_subdirectory(Modules)
add_subdirectory(Web)
add_subdirectory(Applications)
add_subdirectory(Plugins)
add_subdirectory(Tools)
add_subdirectory(Documentation)

# Install rules need to be in a separate directory
add_subdirectory(CMake/InstallRules)
