project(vdfTrackOracleArchiveSource)

set(vdfTrackOracleArchiveSource_Sources
  vdfTrackOracleArchiveSourcePlugin.cxx
  vdfTrackOracleTrackDataSource.cxx
)

if(TARGET kwiver::track_oracle)
  list(APPEND vdfTrackOracleArchiveSource_Sources
    vdfTrackOracleEventDataSource.cxx
  )
endif()

vg_add_qt_plugin(${PROJECT_NAME} ${vdfTrackOracleArchiveSource_Sources})

target_link_libraries(${PROJECT_NAME}
  vvTrackOracle
  vgDataFramework
)

install_plugin_targets(${PROJECT_NAME})
