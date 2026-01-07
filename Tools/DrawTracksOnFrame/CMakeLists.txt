project(drawTracksOnFrame)

add_executable(${PROJECT_NAME} drawTracksOnFrame.cxx)

if(VISGUI_ENABLE_GDAL)
  set(vtkVgIO_LIBRARY vtkVgIO)
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  vgDataFramework
  ${vtkVgIO_LIBRARY}
  vtkVgModelView
  vtkVgCore
  qtExtensionsHeaders
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkGUISupportQt
  vtkInteractionStyle
)

install_executable_target(${PROJECT_NAME} Tools)
