project(drawTracksOnFrame)

vg_include_library_sdk_directories(qtExtensions)

add_executable(${PROJECT_NAME} drawTracksOnFrame.cxx)

if(VISGUI_ENABLE_GDAL)
  set(vtkVgIO_LIBRARY vtkVgIO)
endif()

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  vgDataFramework
  ${vtkVgIO_LIBRARY}
  vtkVgModelView
  vtkVgCore
  LINK_LIBRARIES LINK_PRIVATE
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkGUISupportQt
  vtkInteractionStyle
  ${QT_LIBRARIES}
)

install_executable_target(${PROJECT_NAME} Tools)
