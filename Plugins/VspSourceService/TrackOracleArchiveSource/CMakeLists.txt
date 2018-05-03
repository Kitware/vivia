project(vsTrackOracleArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vspSourceUtil)

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
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
