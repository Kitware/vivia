project(vsContextViewerPlugin)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

vg_include_library_sdk_directories(
  vspUserInterface
  vtkVgSceneGraph
  vtkVgQtUtil
  vtkVgQtSceneUtil
  QtVgCommon
)

set(vsContextViewerPlugin_Sources
  vsContextViewer.cxx
  vsContextViewerPlugin.cxx
)

set(vsContextViewerPlugin_MocHeaders
  vsContextViewer.h
  vsContextViewerPlugin.h
)

qt5_wrap_cpp(vsContextViewerPlugin_MocSources
  ${vsContextViewerPlugin_MocHeaders}
)

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsContextViewerPlugin_Sources}
  ${vsContextViewerPlugin_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  vtkVgSceneGraph
  vtkVgQtSceneUtil
  vtkVgQtUtil
  qtVgCommon
  vtkGUISupportQt
)

install_plugin_targets(${PROJECT_NAME})
