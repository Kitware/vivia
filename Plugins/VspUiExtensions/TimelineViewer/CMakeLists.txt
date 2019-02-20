project(vsTimelineViewerPlugin)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

vg_include_library_sdk_directories(vspUserInterface
  vtkVgQtUtil
  QtVgCommon
)

set(vsTimelineViewerPlugin_Sources
  vsTimelineSelectionCallback.cxx
  vsTimelineViewer.cxx
  vsTimelineViewerPlugin.cxx
)

set(vsTimelineViewerPlugin_MocHeaders
  vsTimelineViewer.h
  vsTimelineViewerPlugin.h
)

qt5_wrap_cpp(vsTimelineViewerPlugin_MocSources
  ${vsTimelineViewerPlugin_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsTimelineViewerPlugin_Sources}
  ${vsTimelineViewerPlugin_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  vtkVgQtUtil
  qtVgCommon
  vtkGUISupportQt
  vtkChartsCore
  vtkViewsContext2D
)

install_plugin_targets(${PROJECT_NAME})
