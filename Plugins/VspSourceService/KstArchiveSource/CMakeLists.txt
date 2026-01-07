project(vsKstArchiveSource)

set(vsKstArchiveSource_Sources
  vsKstArchiveSourcePlugin.cxx
  vsKstDescriptorArchiveSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsKstArchiveSource_Sources})

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
  vvIO
)

install_plugin_targets(${PROJECT_NAME})
