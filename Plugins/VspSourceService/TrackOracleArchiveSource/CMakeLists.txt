project(vsTrackOracleArchiveSource)

if(TARGET kwiver::track_oracle)
  include_directories(SYSTEM ${KWIVER_INCLUDE_DIRS})
  add_definitions(-DKWIVER_TRACK_ORACLE)
else()
  include_directories(SYSTEM ${VIDTK_INCLUDE_DIRS})
endif()

set(vsTrackOracleArchiveSource_Sources
  vsTrackOracleArchiveSourcePlugin.cxx
  vsTrackOracleDescriptorArchiveSource.cxx
  vsTrackOracleTrackArchiveSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsTrackOracleArchiveSource_Sources})

if (TARGET kwiver::track_oracle)
  target_link_libraries(${PROJECT_NAME}
    kwiver::track_oracle
    kwiver::track_oracle_file_formats
  )
else()
  target_link_libraries(${PROJECT_NAME}
    track_oracle
    track_oracle_file_formats
  )
endif()

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
