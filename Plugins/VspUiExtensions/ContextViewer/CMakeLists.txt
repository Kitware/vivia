project(vsContextViewerPlugin)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vsContextViewerPlugin_Sources
  vsContextViewer.cxx
  vsContextViewerPlugin.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsContextViewerPlugin_Sources})

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  vtkVgSceneGraph
  vtkVgQtSceneUtil
  vtkVgQtUtil
  qtVgCommon
  vtkGUISupportQt
)

install_plugin_targets(${PROJECT_NAME})
