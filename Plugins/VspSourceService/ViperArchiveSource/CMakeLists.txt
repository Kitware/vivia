project(vsViperArchiveSource)

include_directories(SYSTEM
  ${VIDTK_INCLUDE_DIRS}
)

set(vsViperArchiveSource_Sources
  vsViperArchiveImportOptionsDialog.cxx
  vsViperArchiveSourcePlugin.cxx
  vsViperArchiveSource.cxx
  vsViperDebug.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsViperArchiveSource_Sources})

target_link_libraries(${PROJECT_NAME}
  track_xgtf
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
