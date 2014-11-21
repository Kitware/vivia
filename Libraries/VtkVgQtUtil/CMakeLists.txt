project(vtkVgQtUtil)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

vg_include_library_sdk_directories(
  vtkVgCore
)

set(vtkVgQtUtilSources
  vtkVgQtAdapt.cxx
  vtkVgQtUtil.cxx
)

set(vtkVgQtUtilInstallHeaders
  vtkVgQtAdapt.h
  vtkVgQtUtil.h
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  vtkGUISupportQt
  vtkRenderingFreeTypeOpenGL
)

# Build and link library.
vtk_module_library(${PROJECT_NAME}
  ${vtkVgQtUtilSources}
)

vg_add_dependencies(${PROJECT_NAME}
  LINK_LIBRARIES
  LINK_PUBLIC
  ${QT_LIBRARIES}
)

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgQtUtilInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgQtUtil)

vg_wrap_vtk_library(${PROJECT_NAME})
