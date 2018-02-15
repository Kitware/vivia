cmake_minimum_required(VERSION 2.8)

project(mrjTranslator)

set(srcs MrjTranslator.cxx
)

add_executable(${PROJECT_NAME} ${srcs})

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vtkVgCore
  LINK_LIBRARIES LINK_PRIVATE
  vtkVgCore
  vtkCommonCore
  vtkCommonSystem
  vtkFiltersGeneral
  vtkIOCore
  vtkIOImage
  vtkImagingCore
)

install_executable_target(${PROJECT_NAME} Tools)
