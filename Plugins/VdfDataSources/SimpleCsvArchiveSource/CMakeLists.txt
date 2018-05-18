project(vdfSimpleCsvArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vgDataFramework)

set(vdfSimpleCsvArchiveSource_Sources
  vdfSimpleCsvArchiveSourcePlugin.cxx
  vdfSimpleCsvTrackArchiveSource.cxx
)

set(vdfSimpleCsvArchiveSource_MocHeaders
  vdfSimpleCsvArchiveSourcePlugin.h
  vdfSimpleCsvTrackArchiveSource.h
)

qt4_wrap_cpp(vdfSimpleCsvArchiveSource_MocSources
             ${vdfSimpleCsvArchiveSource_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vdfSimpleCsvArchiveSource_Sources}
  ${vdfSimpleCsvArchiveSource_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vgDataFramework
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
