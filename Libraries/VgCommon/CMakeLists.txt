project(vgCommon)

find_package(Git REQUIRED)

set(vgCommonSources
  vgAttributeSet.cxx
  vgActivityType.cxx
  vgEntityType.cxx
  vgEventType.cxx
  vgFingerprint.cxx
  vgGeodesy.cxx
)

set(vgCommonInstallHeaders
  vgAbstractFactory.h
  vgActivityType.h
  vgAttributeSet.h
  vgCheckArg.h
  vgEntityType.h
  vgEventType.h
  vgFingerprint.h
  vgFlags.h
  vgGeodesy.h
  vgGeoTypes.h
  vgMatrix.h
  vgNamespace.h
  vgPointerInt.h
  vgRange.h
  vgStringUtils.h
  vgTimeStamp.h
  vgTrackType.h
  vgTypes.h
  vgUtil.h
)

set(vgCommonWrapObjects
  vgGeocodedPoly
  BY_VALUE
  vgGeocodedCoordinate
  vgGeoRawCoordinate
  vgGeoSystem
)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJ4_INCLUDE_DIR}
)

if ("$Format:$" STREQUAL "")
  set(GIT_SHA1   "$Format:%H$")
  set(GIT_STATUS "<generated tarball>")

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vgFingerprint.cxx.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vgFingerprint.cxx"
    @ONLY)
else ()
  add_custom_target(update_fingerprint
    ${CMAKE_COMMAND}
    -D GIT_EXECUTABLE:STRING=${GIT_EXECUTABLE}
    -D GIT_STATUS_WORKING_DIR:PATH="${CMAKE_SOURCE_DIR}"
    -D GIT_STATUS_INPUT_FILE:PATH="${CMAKE_CURRENT_SOURCE_DIR}/vgFingerprint.cxx.in"
    -D GIT_STATUS_OUTPUT_FILE:PATH="${CMAKE_CURRENT_BINARY_DIR}/vgFingerprint.cxx"
    -P ${CMAKE_SOURCE_DIR}/CMake/GetGitStatus.cmake
    COMMENT "Generating build identification"
  )
  set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/vgFingerprint.cxx"
    PROPERTIES GENERATED TRUE SKIP_AUTOGEN TRUE
  )
endif ()

vg_add_library(${PROJECT_NAME}
  ${vgCommonSources}
  "${CMAKE_CURRENT_BINARY_DIR}/vgFingerprint.cxx"
)

target_include_directories(${PROJECT_NAME}Headers SYSTEM
  INTERFACE ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GeographicLib::GeographicLib
  ${PROJ4_LIBRARY}
)

# Tell CMake that building vgCommon depends on the target that creates the
# generated vgFingerprint.cxx source.
add_dependencies(${PROJECT_NAME} update_fingerprint)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vgCommonInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VgCommon)

vg_wrap_library(${PROJECT_NAME}
  TYPESYSTEM typesystem.xml
  OBJECTS ${vgCommonWrapObjects}
  HEADERS ${vgCommonInstallHeaders}
)
