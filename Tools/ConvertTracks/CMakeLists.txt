project(convertTracks)

add_executable(${PROJECT_NAME} convertTracks.cxx)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  vgDataFramework
  vvIO
  qtExtensionsHeaders
)

install_executable_target(${PROJECT_NAME} Tools)
