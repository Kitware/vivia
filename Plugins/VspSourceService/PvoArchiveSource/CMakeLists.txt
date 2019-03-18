project(vsPvoArchiveSource)

set(vsPvoArchiveSource_Sources
  vsPvoArchiveSourcePlugin.cxx
  vsPvoDescriptorArchiveSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsPvoArchiveSource_Sources})

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
