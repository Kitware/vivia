project(mrjTranslator)

add_executable(${PROJECT_NAME} MrjTranslator.cxx)

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
