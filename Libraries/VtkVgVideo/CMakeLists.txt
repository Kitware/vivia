project(vtkVgVideo)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vtkVgVideoSources
  vtkVgAdaptImage.cxx
  vtkVgKWAReader.cxx
  vtkVgKwaVideoSource.cxx
)

set(vtkVgVideoInstallHeaders
  vtkVgAdaptImage.h
  vtkVgKWAReader.h
  vtkVgKwaVideoSource.h
)

set_source_files_properties(
  vtkVgAdaptImage
  PROPERTIES WRAP_EXCLUDE TRUE
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  vtkCommonCore
  vtkImagingCore
  vtkIOImage
  vtkVgCore
)

vtk_module_library(${PROJECT_NAME}
  ${vtkVgVideoSources}
  ${vtkVgVideoMocSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vtkVgCore
  PRIVATE_INTERFACE_TARGETS
  vgVideo
  LINK_LIBRARIES
  LINK_PRIVATE
  vgl_algo
)

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgVideoInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgVideo)

vg_wrap_vtk_library(${PROJECT_NAME})
