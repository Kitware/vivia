project(vvFakeQueryService)

set(vvFakeQueryService_Sources
  vvFakeQueryServerChooser.cxx
  vvFakeQueryServicePlugin.cxx
  vvFakeQuerySession.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vvFakeQueryService_Sources})

target_link_libraries(${PROJECT_NAME}
  vvIO
  vvWidgets
  qtVgCommon
  qtExtensions
)

install_plugin_targets(${PROJECT_NAME})
