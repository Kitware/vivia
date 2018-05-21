project(vdfTrackOracleArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vgDataFramework)

if(TARGET kwiver::track_oracle)
  include_directories(SYSTEM ${KWIVER_INCLUDE_DIRS})
  add_definitions(-DKWIVER_TRACK_ORACLE)
else()
  include_directories(SYSTEM ${VIDTK_INCLUDE_DIRS})
endif()

set(vdfTrackOracleArchiveSource_Sources
  vdfTrackOracleArchiveSourcePlugin.cxx
  vdfTrackOracleTrackArchiveSource.cxx
)

set(vdfTrackOracleArchiveSource_MocHeaders
  vdfTrackOracleArchiveSourcePlugin.h
  vdfTrackOracleTrackArchiveSource.h
)

qt4_wrap_cpp(vdfTrackOracleArchiveSource_MocSources
             ${vdfTrackOracleArchiveSource_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vdfTrackOracleArchiveSource_Sources}
  ${vdfTrackOracleArchiveSource_MocSources}
)

if(TARGET kwiver::track_oracle)
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
  vgDataFramework
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
