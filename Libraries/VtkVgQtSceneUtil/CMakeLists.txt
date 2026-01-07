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

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vtkVgSceneGraph
  qtExtensions
  PRIVATE
  vil_io
  vnl_io
  vgl_algo
)

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgQtSceneUtilInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgQtSceneUtil)
