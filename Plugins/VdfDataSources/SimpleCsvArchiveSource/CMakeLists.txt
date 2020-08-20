project(vdfSimpleCsvArchiveSource)

set(vdfSimpleCsvArchiveSource_Sources
  vdfSimpleCsvArchiveSourcePlugin.cxx
  vdfSimpleCsvTrackArchiveSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vdfSimpleCsvArchiveSource_Sources})

target_link_libraries(${PROJECT_NAME}
  vgDataFramework
)

install_plugin_targets(${PROJECT_NAME})
