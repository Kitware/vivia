project(vsEventCreationTools)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vsEventCreationTools_Sources
  vsEventCreationToolsInterface.cxx
  vsEventCreationToolsPlugin.cxx
)

set(vsEventCreationTools_ResSources
  icons.qrc
)

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsEventCreationTools_Sources}
  ${vsEventCreationTools_ResSources}
)

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  vtkGUISupportQt
)

install_plugin_targets(${PROJECT_NAME})
