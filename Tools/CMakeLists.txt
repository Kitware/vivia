include(${qtExtensions_SOURCE_DIR}/CMake/Modules/UseQtExtensions.cmake)

if(VISGUI_ENABLE_VVIO AND VISGUI_ENABLE_VIDTK)
  add_subdirectory(ConvertTracks)
endif()

add_subdirectory(FixHeaderGuards)
add_subdirectory(MrjTranslator)
add_subdirectory(QSettingsTool)

option(VISGUI_ENABLE_KML_WRITER "Build kml writer utility" OFF)
if(KML_WRITER)
  add_subdirectory(KmlWriter)
endif()
