project(vtkVwCore)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
  ${LIBJSON_INCLUDE_DIR}
  ${PYTHON_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vtkVwCoreSources
  vtkVwVideo.cxx
)

set(vtkVwCoreInstallHeaders
  vtkVwVideo.h
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS vtkCommonCore
  EXCLUDE_FROM_WRAP_HIERARCHY
)
vtk_module_library(${PROJECT_NAME} ${vtkVwCoreSources})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  vgVideo
  vtkVgCore
  vvIO
  ${LIBJSON_LIBRARY}
  ${PYTHON_LIBRARIES}
)

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVwCoreInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/vtkVwCore)

vg_wrap_vtk_library(${PROJECT_NAME})
