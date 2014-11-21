project(vsEventCreationTools)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

vg_include_library_sdk_directories(vspUserInterface)

set(vsEventCreationTools_Sources
  vsEventCreationToolsInterface.cxx
  vsEventCreationToolsPlugin.cxx
)

set(vsEventCreationTools_MocHeaders
  vsEventCreationToolsInterface.h
  vsEventCreationToolsPlugin.h
)

set(vsEventCreationTools_Resources
  icons.qrc
)

qt4_wrap_cpp(vsEventCreationTools_MocSources
  ${vsEventCreationTools_MocHeaders}
)
qt4_add_resources(vsEventCreationTools_ResSources
  ${vsEventCreationTools_Resources}
)

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsEventCreationTools_Sources}
  ${vsEventCreationTools_MocSources}
  ${vsEventCreationTools_ResSources}
)

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  ${QT_LIBRARIES}
  vtkGUISupportQt
)

install_plugin_targets(${PROJECT_NAME})
