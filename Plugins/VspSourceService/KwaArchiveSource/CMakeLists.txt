project(vsKwaArchiveSource)

set(vsKwaArchiveSource_Sources
  vsKwaArchiveSourcePlugin.cxx
  vsKwaVideoArchiveSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsKwaArchiveSource_Sources})

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
