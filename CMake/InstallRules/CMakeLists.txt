# Bundle cpack
include(InstallRequiredSystemLibraries)

set(DIRS ${global_library_search_paths} ${Boost_LIBRARY_DIRS})

set(BINARY_NAME)
set(BINARY_DIR)
if(VISGUI_ENABLE_VSPLAY)
  set(BINARY_NAME vsPlay)
  set(BINARY_DIR  ${vsPlay_BINARY_DIR})
elseif(VISGUI_ENABLE_VPVIEW)
  set(BINARY_NAME vpView)
  set(BINARY_DIR  ${vpView_BINARY_DIR})
  set(DIRS ${DIRS} ${CPPDB_LIB_DIR})
else()
  message(FATAL_ERROR "Installer only supports vpView and vsPlay at this time")
endif()

set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/${BINARY_NAME}")
if(WIN32)
  set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/${BINARY_NAME}.exe")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${BINARY_NAME}.exe")
endif(WIN32)
if(APPLE)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${BINARY_NAME}.app")

  # Write an empty qt.conf file. Without this, Qt will spew warnings about
  # ambiguous library references when running the Mac app bundle on a system
  # with its own Qt installation.
  set(QT_CONF_DIR ${BINARY_NAME}.app/Contents/Resources)
  INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${QT_CONF_DIR}/qt.conf\" \"\")
  ")
endif(APPLE)



if (NOT VISGUI_DISABLE_FIXUP_BUNDLE)
install(CODE "
  message(\"Running fixup_bundle\")
  include(BundleUtilities)
  fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
")
endif()

set(CPACK_PACKAGE_NAME "VisGUI")
set(CPACK_PACKAGE_VENDOR "Kitware")
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
# Currently the versions match irrespective of the application
set(CPACK_PACKAGE_VERSION_MAJOR ${VISGUI_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VISGUI_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VISGUI_VERSION_PATCH})

set(CPACK_PACKAGE_EXECUTABLES "${BINARY_NAME}" "VisGUI ${BINARY_NAME}")
include(CPack)
