project(convertTracks)

vg_include_library_sdk_directories(qtExtensions)

add_executable(${PROJECT_NAME} convertTracks.cxx)

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  vgDataFramework
  vvIO
)

install_executable_target(${PROJECT_NAME} Tools)
