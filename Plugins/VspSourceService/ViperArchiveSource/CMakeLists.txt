project(vsViperArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vspSourceUtil)
include_directories(SYSTEM
  ${VIDTK_INCLUDE_DIRS}
)

set(vsViperArchiveSource_Sources
  vsViperArchiveImportOptionsDialog.cxx
  vsViperArchiveSourcePlugin.cxx
  vsViperArchiveSource.cxx
  vsViperDebug.cxx
)

set(vsViperArchiveSource_MocHeaders
  vsViperArchiveImportOptionsDialog.h
  vsViperArchiveSourcePlugin.h
  vsViperArchiveSource.h
  vsViperArchiveSourcePrivate.h
)

set(vsViperArchiveSource_UI
  viperOptions.ui
)

qt4_wrap_cpp(vsViperArchiveSource_MocSources
             ${vsViperArchiveSource_MocHeaders})

qt4_wrap_ui(vsViperArchiveSource_UiSources ${vsViperArchiveSource_UI})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsViperArchiveSource_Sources}
  ${vsViperArchiveSource_MocSources}
  ${vsViperArchiveSource_UiSources}
)

target_link_libraries(${PROJECT_NAME}
  track_xgtf
  vspSourceUtil
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
