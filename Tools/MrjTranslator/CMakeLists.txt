cmake_minimum_required(VERSION 2.8)

project(mrjTranslator)

set(srcs MrjTranslator.cxx
)

add_executable(${PROJECT_NAME} ${srcs})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  vtkVgCore
  vtkCommonCore
  vtkCommonSystem
  vtkFiltersGeneral
  vtkIOCore
  vtkIOImage
  vtkImagingCore
)

install_executable_target(${PROJECT_NAME} Tools)
