list(APPEND CMAKE_MODULE_PATH ${visGUI_SOURCE_DIR}/CMake/Modules)

find_package(Eigen3 REQUIRED)

find_package(Qt5 5.8.0 REQUIRED COMPONENTS
  Core
  Gui
  Widgets
  Network
  Concurrent
  Xml
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Generally, Boost is built shared, but give an advanced option to find a static build.
option(USE_STATIC_BOOST
  "Find a static build of Boost"
  OFF)
mark_as_advanced(USE_STATIC_BOOST)
set(Boost_USE_STATIC_LIBS ${USE_STATIC_BOOST})

# Boost is required.
find_package(Boost REQUIRED
  COMPONENTS thread system filesystem date_time
)
add_definitions(-DBOOST_ALL_NO_LIB)

set(CMAKE_THREAD_PREFER_PTHREAD 1)
find_package(Threads)
set(Boost_LIBRARIES ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if(VISGUI_ENABLE_VPVIEW OR VISGUI_ENABLE_WEB)
  # libjson is required for vpView and vtkVwCore
  find_package(LIBJSON REQUIRED)
endif()

set(vtk_python_wrap_core)
if(VISGUI_ENABLE_PYTHON)
  set(vtk_python_wrap_core vtkPython vtkWrappingPythonCore)
endif()

set(vtk_testing_support)
if(BUILD_TESTING)
  set(vtk_testing_support vtkTestingRendering)
endif()

# VTK OpenGL components
set(VTK_OPENGL_RENDERING_COMPONENTS
  vtkRenderingOpenGL
  vtkRenderingContextOpenGL
  vtkRenderingVolumeOpenGL
)

# VTK is required
set(VTK_REQUIRED_COMPONENTS
  # Core components
  vtkChartsCore
  vtkCommonCore
  vtkFiltersCore
  vtkImagingCore
  vtkInfovisCore
  vtkIOCore
  vtkViewsCore
  # Additional features
  ${vtk_python_wrap_core}
  vtkCommonColor
  vtkCommonDataModel
  vtkCommonExecutionModel
  vtkCommonMath
  vtkCommonSystem
  vtkFiltersExtraction
  vtkFiltersGeneral
  vtkFiltersHybrid
  vtkFiltersModeling
  vtkFiltersSources
  vtkFiltersStatistics
  vtkFiltersTexture
  vtkImagingHybrid
  vtkImagingStencil
  vtkInfovisLayout
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkIOImage
  vtkIOLegacy
  vtkIOXML
  vtkRenderingAnnotation
  vtkRenderingContext2D
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkRenderingLabel
  vtksys
  ${vtk_testing_support}
  vtkViewsContext2D
  vtkViewsInfovis
  # GUI support
  vtkGUISupportQt
)

# We need ParaView for VisGUI-Web
if(VISGUI_ENABLE_WEB)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
  if (NOT VTK_DIR AND EXISTS "${VTK_CONFIG_FILE}")
    get_filename_component(VTK_DIR "${VTK_CONFIG_FILE}" PATH)
  elseif(VTK_DIR)
    message("Using VTK from ${VTK_DIR}")
  endif()
endif()

find_package(VTK 8.0 NO_MODULE REQUIRED COMPONENTS ${VTK_REQUIRED_COMPONENTS})
include(${VTK_USE_FILE})

# We need this definition to be set regardless
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

include(vtkExternalModuleMacros)
if(VISGUI_ENABLE_PYTHON)
  include(vtkPythonWrapping)
  set(VTK_WRAP_TCL OFF)
  find_file(VTK_WRAP_HINTS
    NAMES hints
    PATHS ${VTK_CMAKE_DIR}/../Wrapping/Tools
    NO_CMAKE_FIND_ROOT_PATH
  )
endif()

find_package(VXL REQUIRED)

# DO NOT use UseVXL.cmake; its documentation stuff is broken
include_directories(SYSTEM ${VXL_VCL_INCLUDE_DIRS} ${VXL_CORE_INCLUDE_DIRS})
link_directories(${VXL_LIBRARY_DIR})

if (VISGUI_ENABLE_VIDTK)
  find_package(vidtk REQUIRED)
  add_definitions(-DVISGUI_USE_VIDTK)

  if (VISGUI_ENABLE_SUPER3D)
    add_definitions(-DVISGUI_USE_SUPER3D)
  endif()

  # This is a fix for a change in VIDTK. It is required or else
  # class definitions and other code would be different when seen
  # by VISGUI.
  add_definitions(-DUUIDABLE)
else()
  if (VISGUI_ENABLE_SUPER3D)
    message(ERROR_FATAL "Enabling vivia_super3d requires enabling vidtk")
  endif()
endif()

if (VISGUI_ENABLE_KWIVER)
  find_package(kwiver REQUIRED)
  add_definitions(-DVISGUI_USE_KWIVER)
endif()

# QtTesting is required for tests
find_package(QtTesting QUIET)
if(QtTesting_FOUND)
  find_multi_configuration_library(QtTesting QtTesting ${QtTesting_LIBRARY_DIR})
  set(QtTesting_LIBRARIES ${QtTesting_LIBRARY})
else()
   message("QtTesting was not found; GUI testing will not be enabled")
endif()

find_package(qtExtensions REQUIRED)

find_package(PROJ4 REQUIRED)
find_package(KML REQUIRED)

if(VISGUI_ENABLE_GDAL)
  find_package(GDAL REQUIRED)
  add_definitions(-DVISGUI_USE_GDAL)
endif()

find_package(GeographicLib REQUIRED)

if(VISGUI_ENABLE_SUPER3D)
  find_package(super3d REQUIRED)
  find_package(maptk REQUIRED)
  find_package(OpenCV REQUIRED)
endif()
