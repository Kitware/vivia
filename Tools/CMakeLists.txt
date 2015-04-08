include(${qtExtensions_SOURCE_DIR}/CMake/Modules/UseQtExtensions.cmake)

add_subdirectory(ConvertTracks)
add_subdirectory(DrawTracksOnFrame)
add_subdirectory(FixHeaderGuards)
add_subdirectory(MrjTranslator)
add_subdirectory(QSettingsTool)

option(VISGUI_ENABLE_KML_WRITER "Build kml writer utility" OFF)
if(KML_WRITER)
  add_subdirectory(KmlWriter)
endif()
