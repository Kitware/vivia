project(vtkVgQtUtil)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vtkVgQtUtilSources
  vtkVgQtUtil.cxx
)

set(vtkVgQtUtilInstallHeaders
  vtkVgQtUtil.h
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkGUISupportQt
  vtkInteractionStyle
)

# Build and link library.
vtk_module_library(${PROJECT_NAME}
  ${vtkVgQtUtilSources}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vtkVgCoreHeaders
)

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgQtUtilInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgQtUtil)

vg_wrap_vtk_library(${PROJECT_NAME})
