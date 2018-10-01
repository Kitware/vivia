project(vtkVgCore)

# Requires VTK and VXL (using VIL of VXL).
include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

# Source files.
set(vtkVgCoreSrcs
  vtkVgContourOperatorManager.cxx
  vtkVgScalars.cxx
  vtkVgTemporalFilters.cxx
  vtkVgTimeStamp.cxx
  vtkVgTrack.cxx
  vtkVgTrackTypeRegistry.cxx
)

set(vtkVgCoreInstallHeaders
  vtkVgContourOperatorManager.h
  vtkVgScalars.h
  vtkVgTemporalFilters.h
  vtkVgTimeStamp.h
  vtkVgTrack.h
  vtkVgTrackTypeRegistry.h
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  vtkCommonCore
  vtkInteractionStyle
  vtkRenderingFreeType
)

# Build and link library.
vtk_module_library(${PROJECT_NAME} ${vtkVgCoreSrcs})

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vgCommon
)

install_headers(${vtkVgCoreInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgCore)
