# Name of the project.
project(vtkVgIO)

# GDAL is required for NITF and JP2 support
find_package(GDAL REQUIRED)

if(NOT VTK_USE_SYSTEM_JPEG)
  add_definitions(-DVTK_JPEG_INCLUDE_JERROR)
endif(NOT VTK_USE_SYSTEM_JPEG)

# Requires VTK and GDAL.
include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
  ${GDAL_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
)

# Source files.
set(vtkVgIOSrcs
  vtkGDALReader.cxx
  vtkVgGDALReader.cxx
  vtkVgNitfEngrda.cxx
  vtkVgNitfMetaDataParser.cxx
)

# Install headers
set(vtkVgIOInstallHeaders
  vtkGDALReader.h
  vtkVgGDALReader.h
  vtkVgNitfEngrda.h
  vtkVgNitfMetaDataParser.h
)

set_source_files_properties(
  vtkVgNitfEngrda
  vtkVgNitfMetaDataParser
  PROPERTIES WRAP_EXCLUDE TRUE
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkIOCore
  vtkIOLegacy
)

vtk_module_library(${PROJECT_NAME}
  ${vtkVgIOSrcs}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vgCommon
  vtkVgCore
  PRIVATE
  ${GDAL_LIBRARY}
  ${Boost_LIBRARIES}
)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgIOInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgIO)

vg_wrap_vtk_library(${PROJECT_NAME})
