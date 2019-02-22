project(qtVgCommon)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

set(sources
    vgColor.cxx
    vgGeoUtil.cxx
    vgPluginLoader.cxx
    vgRegionKeyframe.cxx
    vgSwatchCache.cxx
    vgTimeBase.cxx
    "${CMAKE_CURRENT_BINARY_DIR}/vgPluginPaths.cxx"
)

set(qtVgCommonInstallHeaders
  qtMap.h
  qtMapImpl.h
  vgColor.h
  vgDebug.h
  vgFileDialog.h
  vgGeoUtil.h
  vgPluginLoader.h
  vgRegionKeyframe.h
  vgSwatchCache.h
  vgTimeBase.h
  vgTime.h
  vgTimeMap.h
  vgUnixTime.h
)

configure_file(
  vgPluginPaths.cxx.in
  "${CMAKE_CURRENT_BINARY_DIR}/vgPluginPaths.cxx"
)

vg_add_library(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME}Headers
  INTERFACE
  vgCommonHeaders
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  qtExtensions
  vgCommon
)

install_library_targets(${PROJECT_NAME})
install_headers(${qtVgCommonInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/QtVgCommon)
