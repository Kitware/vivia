project(vvKwiver)

# Use viame-3 install for Qt5, VTK, and KWIVER since they're all built against Qt5
set(VIAME3_INSTALL_DIR "/home/local/KHQ/matt.dawkins/Dev/viame-3/build/install")
include_directories(BEFORE SYSTEM ${VIAME3_INSTALL_DIR}/include)

# Oh, God, fix this!!! (https://github.com/Kitware/kwiver/issues/247)
# Use SYSTEM to suppress warnings from KWIVER headers
include_directories(SYSTEM ${KWIVER_INCLUDE_DIRS})

vg_include_library_sdk_directories(
  TARGET ${PROJECT_NAME} INTERFACE vvData vvIO
)

set(vvKwiverInstallHeaders
  vvAdaptKwiver.h
)

add_library(${PROJECT_NAME} vvAdaptKwiver.cxx)

target_link_libraries(${PROJECT_NAME} PUBLIC
  vvIO
  kwiver::vital
  Qt5::Core
)

install_library_targets(${PROJECT_NAME})
install_headers(${vvKwiverInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvVidtk)
