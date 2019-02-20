project(vsFakeStreamSourceFactory)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vspSourceUtil)

set(vsFakeStreamSourceFactory_Sources
  vsFakeStreamControl.cxx
  vsFakeStreamFactory.cxx
  vsFakeStreamSourceFactoryPlugin.cxx
  vsFakeStreamSource.cxx
)

set(vsFakeStreamSourceFactory_MocHeaders
  vsFakeStreamControl.h
  vsFakeStreamSourceFactoryPlugin.h
  vsFakeStreamSource.h
  vsFakeStreamSourcePrivate.h
)

set(vsFakeStreamSourceFactory_UI
  fakeStreamControl.ui
)

qt5_wrap_cpp(vsFakeStreamSourceFactory_MocSources
             ${vsFakeStreamSourceFactory_MocHeaders})

qt5_wrap_ui(vsFakeStreamSourceFactory_UiSources
            ${vsFakeStreamSourceFactory_UI})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsFakeStreamSourceFactory_Sources}
  ${vsFakeStreamSourceFactory_MocSources}
  ${vsFakeStreamSourceFactory_UiSources}
)

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
