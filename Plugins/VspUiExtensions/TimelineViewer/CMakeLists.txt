project(vsTimelineViewerPlugin)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vsTimelineViewerPlugin_Sources
  vsTimelineSelectionCallback.cxx
  vsTimelineViewer.cxx
  vsTimelineViewerPlugin.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsTimelineViewerPlugin_Sources})

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  vtkVgQtUtil
  qtVgCommon
  vtkGUISupportQt
  vtkChartsCore
  vtkViewsContext2D
)

install_plugin_targets(${PROJECT_NAME})
