project(vvTrackOracle)

set(vvTrackOracle_InstallHeaders
  track_oracle_file_format_fwd.h
  track_oracle_utils.h
  visgui_event_type.h
  visgui_track_type.h
)

add_library(${PROJECT_NAME} INTERFACE)

if(TARGET kwiver::track_oracle)
  target_compile_definitions(${PROJECT_NAME}
    INTERFACE
    KWIVER_TRACK_ORACLE
  )
  target_include_directories(${PROJECT_NAME}
    SYSTEM INTERFACE
    ${KWIVER_INCLUDE_DIRS}
  )
  target_link_libraries(${PROJECT_NAME}
    INTERFACE
    kwiver::track_oracle
    kwiver::track_oracle_file_formats
    kwiver::vital
  )
else()
  target_include_directories(${PROJECT_NAME}
    SYSTEM INTERFACE
    ${VIDTK_INCLUDE_DIRS}
  )
  target_link_libraries(${PROJECT_NAME}
    INTERFACE
    track_oracle
    track_oracle_file_formats
  )
endif()

install_headers(${vvTrackOracle_InstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvTrackOracle)
