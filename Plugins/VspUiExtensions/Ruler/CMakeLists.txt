project(vsRulerTool)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vsRuler_Sources
  vsLineWidget.cxx
  vsRulerInterface.cxx
  vsRulerPlugin.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsRuler_Sources})

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  vtkVgQtUtil
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkGUISupportQt
)

install_plugin_targets(${PROJECT_NAME})
