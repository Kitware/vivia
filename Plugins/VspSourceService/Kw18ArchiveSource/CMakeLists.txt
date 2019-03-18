project(vsKw18ArchiveSource)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${VIDTK_INCLUDE_DIRS}
)

set(vsKw18ArchiveSource_Sources
  vsKw18ArchiveSourcePlugin.cxx
  vsKw18TrackArchiveSource.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsKw18ArchiveSource_Sources})

target_link_libraries(${PROJECT_NAME}
  vvVidtk
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
