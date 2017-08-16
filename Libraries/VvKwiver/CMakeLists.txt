project(vvKwiver)

# Oh, God, fix this!!! (https://github.com/Kitware/kwiver/issues/247)
include_directories(${KWIVER_INCLUDE_DIRS})

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
  ${QT_LIBRARIES}
)

install_library_targets(${PROJECT_NAME})
install_headers(${vvKwiverInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvVidtk)
