cmake_minimum_required(VERSION 2.8)

project(mrj2png)

include_directories(
  ${vtkVgCore_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} main.cxx)

target_link_libraries(${PROJECT_NAME}
  vtkVgCore
  vtkCommon
  vtkFiltering
  vtkIO
  vtkImaging
)

install_executable_target(${PROJECT_NAME} Tools)
