project(vsKw18ArchiveSource)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(
  vvVidtk
  vspSourceUtil
)
include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${VIDTK_INCLUDE_DIRS}
)

set(vsKw18ArchiveSource_Sources
  vsKw18ArchiveSourcePlugin.cxx
  vsKw18TrackArchiveSource.cxx
)

set(vsKw18ArchiveSource_MocHeaders
  vsKw18ArchiveSourcePlugin.h
  vsKw18TrackArchiveSource.h
)

qt5_wrap_cpp(vsKw18ArchiveSource_MocSources ${vsKw18ArchiveSource_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsKw18ArchiveSource_Sources}
  ${vsKw18ArchiveSource_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vvVidtk
  vspSourceUtil
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
