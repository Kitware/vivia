project(vdfTrackOracleArchiveSource)

if(TARGET kwiver::track_oracle)
  include_directories(SYSTEM ${KWIVER_INCLUDE_DIRS})
  add_definitions(-DKWIVER_TRACK_ORACLE)
else()
  include_directories(SYSTEM ${VIDTK_INCLUDE_DIRS})
endif()

set(vdfTrackOracleArchiveSource_Sources
  vdfTrackOracleArchiveSourcePlugin.cxx
  vdfTrackOracleTrackDataSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vdfTrackOracleArchiveSource_Sources})

if(TARGET kwiver::track_oracle)
  target_link_libraries(${PROJECT_NAME}
    kwiver::track_oracle
    kwiver::track_oracle_file_formats
    kwiver::vital
  )
else()
  target_link_libraries(${PROJECT_NAME}
    track_oracle
    track_oracle_file_formats
  )
endif()

target_link_libraries(${PROJECT_NAME}
  vgDataFramework
)

install_plugin_targets(${PROJECT_NAME})
