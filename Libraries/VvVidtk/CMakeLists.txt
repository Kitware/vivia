project(vvVidtk)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${VIDTK_INCLUDE_DIRS}
)

set(vvVidtkInstallHeaders
  vvAdaptVidtk.h
)

add_library(${PROJECT_NAME} vvAdaptVidtk.cxx)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt5::Core
  PRIVATE
  vvData
  vidtk_tracking
)

install_library_targets(${PROJECT_NAME})
install_headers(${vvVidtkInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvVidtk)
