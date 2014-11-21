project(vsTrackOracleArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vspSourceUtil)
include_directories(SYSTEM
  ${VIDTK_INCLUDE_DIRS}
)

set(vsTrackOracleArchiveSource_Sources
  vsTrackOracleArchiveSourcePlugin.cxx
  vsTrackOracleDescriptorArchiveSource.cxx
  vsTrackOracleTrackArchiveSource.cxx
)

set(vsTrackOracleArchiveSource_MocHeaders
  vsTrackOracleArchiveSourcePlugin.h
  vsTrackOracleDescriptorArchiveSource.h
  vsTrackOracleTrackArchiveSource.h
)

qt4_wrap_cpp(vsTrackOracleArchiveSource_MocSources
             ${vsTrackOracleArchiveSource_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsTrackOracleArchiveSource_Sources}
  ${vsTrackOracleArchiveSource_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  track_oracle
  track_oracle_file_formats
  vspSourceUtil
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
