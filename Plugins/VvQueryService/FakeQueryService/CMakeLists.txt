project(vvFakeQueryService)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vvFakeQueryService_SdkTargets
  vvIO
  vvWidgets
  qtVgCommon
  qtExtensions
)

vg_include_library_sdk_directories(${vvFakeQueryService_SdkTargets})

set(vvFakeQueryService_Sources
  vvFakeQueryServerChooser.cxx
  vvFakeQueryServicePlugin.cxx
  vvFakeQuerySession.cxx
)

set(vvFakeQueryService_MocHeaders
  vvFakeQueryServerChooser.h
  vvFakeQueryServicePlugin.h
  vvFakeQuerySession.h
)

set(vvFakeQueryService_UiFiles
  vvFakeQueryServerChooser.ui
)

qt5_wrap_cpp(vvFakeQueryService_MocSources ${vvFakeQueryService_MocHeaders})
qt5_wrap_ui(vvFakeQueryService_UiSources ${vvFakeQueryService_UiFiles})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vvFakeQueryService_Sources}
  ${vvFakeQueryService_MocSources}
  ${vvFakeQueryService_UiSources}
)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
  ${vvFakeQueryService_SdkTargets}
)

install_plugin_targets(${PROJECT_NAME})
