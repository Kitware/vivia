project(vtkVgModelView)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vtkVgModelViewSrcs
  vtkVgModelBase.cxx
  vtkVgTrackFilter.cxx
  vtkVgTrackInfo.cxx
  vtkVgTrackModel.cxx
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  vtkVgCore
)

# Build and link library.
vtk_module_library(${PROJECT_NAME} ${vtkVgModelViewSrcs})

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vtkVgCore
)
