project(vsPvoArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vspSourceUtil)

set(vsPvoArchiveSource_Sources
  vsPvoArchiveSourcePlugin.cxx
  vsPvoDescriptorArchiveSource.cxx
)

set(vsPvoArchiveSource_MocHeaders
  vsPvoArchiveSourcePlugin.h
  vsPvoDescriptorArchiveSource.h
)

qt5_wrap_cpp(vsPvoArchiveSource_MocSources ${vsPvoArchiveSource_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsPvoArchiveSource_Sources}
  ${vsPvoArchiveSource_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
