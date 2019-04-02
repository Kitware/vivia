project(vtkVgQtSceneUtil)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vtkVgQtSceneUtilSrcs
  vtkVgCoordinateTransform.cxx
  vtkVgTerrainSource.cxx
)

set(vtkVgQtSceneUtilInstallHeaders
  vtkVgCoordinateTransform.h
  vtkVgTerrainSource.h
)

# Build and link library.
add_library(${PROJECT_NAME} ${vtkVgQtSceneUtilSrcs})

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vtkVgSceneGraph
  qtExtensions
  LINK_LIBRARIES
  LINK_PRIVATE
  vil_io
  vnl_io
  vgl_algo
)

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgQtSceneUtilInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgQtSceneUtil)
