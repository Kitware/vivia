project(vsRulerTool)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vsRuler_SdkTargets
  vspUserInterface
  vtkVgQtUtil
)

vg_include_library_sdk_directories(${vsRuler_SdkTargets})

set(vsRuler_Sources
  vsLineWidget.cxx
  vsRulerInterface.cxx
  vsRulerPlugin.cxx
)

set(vsRuler_MocHeaders
  vsLineWidget.h
  vsRulerInterface.h
  vsRulerPlugin.h
)

qt4_wrap_cpp(vsRuler_MocSources
  ${vsRuler_MocHeaders}
)

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsRuler_Sources}
  ${vsRuler_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  ${vsRuler_SdkTargets}
  ${QT_LIBRARIES}
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkGUISupportQt
)

install_plugin_targets(${PROJECT_NAME})
