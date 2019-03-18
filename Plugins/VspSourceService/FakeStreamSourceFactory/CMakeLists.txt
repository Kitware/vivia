project(vsFakeStreamSourceFactory)

set(vsFakeStreamSourceFactory_Sources
  vsFakeStreamControl.cxx
  vsFakeStreamFactory.cxx
  vsFakeStreamSourceFactoryPlugin.cxx
  vsFakeStreamSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsFakeStreamSourceFactory_Sources})

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
