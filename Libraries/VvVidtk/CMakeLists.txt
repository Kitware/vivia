project(vvVidtk)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${VIDTK_INCLUDE_DIRS}
)

vg_include_library_sdk_directories(
  TARGET ${PROJECT_NAME} INTERFACE vvData
)

set(vvVidtkInstallHeaders
  vvAdaptVidtk.h
)

add_library(${PROJECT_NAME} vvAdaptVidtk.cxx)

target_link_libraries(${PROJECT_NAME} vidtk_tracking)

install_library_targets(${PROJECT_NAME})
install_headers(${vvVidtkInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvVidtk)
