list(APPEND CMAKE_MODULE_PATH ${visGUI_SOURCE_DIR}/CMake/Modules)

# Boost is required.
find_package(Boost REQUIRED
  COMPONENTS thread signals system filesystem date_time
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
  vtkRenderingContextOpenGL
  vtkRenderingFreeType
  vtkRenderingFreeTypeOpenGL
  vtkRenderingLabel
  vtkRenderingOpenGL
  vtkRenderingVolumeOpenGL
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

find_package(VTK NO_MODULE REQUIRED COMPONENTS ${VTK_REQUIRED_COMPONENTS})
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
include_directories(SYSTEM ${VXL_VCL_INCLUDE_DIR} ${VXL_CORE_INCLUDE_DIR})
link_directories(${VXL_LIBRARY_DIR})

if (VISGUI_ENABLE_VIDTK)
  find_package(vidtk REQUIRED)
  add_definitions(-DVISGUI_USE_VIDTK)

  # This is a fix for a change in VIDTK. It is required or else
  # class definitions and other code would be different when seen
  # by VISGUI.
  add_definitions(-DUUIDABLE)
endif()

# QtTesting is required for tests
find_package(QtTesting QUIET)
if(QtTesting_FOUND)
  find_multi_configuration_library(QtTesting QtTesting ${QtTesting_LIBRARY_DIR})
  set(QtTesting_LIBRARIES ${QtTesting_LIBRARY})
else()
   message("QtTesting was not found; GUI testing will not be enabled")
endif()

set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTXML TRUE)
find_package(Qt4 4.7.0 REQUIRED)

# This adds ${QT_INCLUDE_DIR} to the build (via include_directories)
include(${QT_USE_FILE})

find_package(PROJ4 REQUIRED)
find_package(KML REQUIRED)

if(VISGUI_ENABLE_GDAL)
  find_package(GDAL REQUIRED)
  add_definitions(-DVISGUI_USE_GDAL)
endif()

find_package(geographiclib REQUIRED)
