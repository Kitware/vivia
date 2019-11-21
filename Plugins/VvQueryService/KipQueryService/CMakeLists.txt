project(vvKipQueryService)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vvKipQueryService_SdkTargets
  vvKwiver
  vvIO
  vvWidgets
  qtVgCommon
  qtExtensions
)

vg_include_library_sdk_directories(${vvKipQueryService_SdkTargets})

# Oh, God, fix this!!! (https://github.com/Kitware/kwiver/issues/247)
include_directories(${KWIVER_INCLUDE_DIRS})

set(vvKipQueryService_Sources
  vvKipQueryServerChooser.cxx
  vvKipQueryServicePlugin.cxx
  vvKipQuerySession.cxx
)

set(vvKipQueryService_MocHeaders
  vvKipQueryServerChooser.h
  vvKipQueryServicePlugin.h
  vvKipQuerySession.h
)

set(vvKipQueryService_UiFiles
  vvKipQueryServerChooser.ui
)

qt4_wrap_cpp(vvKipQueryService_MocSources ${vvKipQueryService_MocHeaders})
qt4_wrap_ui(vvKipQueryService_UiSources ${vvKipQueryService_UiFiles})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vvKipQueryService_Sources}
  ${vvKipQueryService_MocSources}
  ${vvKipQueryService_UiSources}
)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
  kwiver::sprokit_pipeline
  kwiver::kwiver_adapter
  kwiver::vital_vpm
  kwiver::vital
  ${vvKipQueryService_SdkTargets}
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
