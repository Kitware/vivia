project(vsKwaArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vspSourceUtil)

set(vsKwaArchiveSource_Sources
  vsKwaArchiveSourcePlugin.cxx
  vsKwaVideoArchiveSource.cxx
)

set(vsKwaArchiveSource_MocHeaders
  vsKwaArchiveSourcePlugin.h
  vsKwaVideoArchiveSource.h
)

qt5_wrap_cpp(vsKwaArchiveSource_MocSources ${vsKwaArchiveSource_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsKwaArchiveSource_Sources}
  ${vsKwaArchiveSource_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
