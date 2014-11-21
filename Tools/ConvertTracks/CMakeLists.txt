project(convertTracks)

include_directories(SYSTEM
  ${VIDTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

vg_include_library_sdk_directories(qtExtensions)

add_executable(${PROJECT_NAME} convertTracks.cxx)

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  vvVidtk
  vvIO
  LINK_LIBRARIES LINK_PRIVATE
  vidtk_tracking
  vul
  ${QT_LIBRARIES}
)

install_executable_target(${PROJECT_NAME} Tools)
