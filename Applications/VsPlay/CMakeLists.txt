project(vsPlay)

###############################################################################

# BEGIN user configurable build options

if(NOT VISGUI_ENABLE_VSPLAY)
  # If vsPlay is disabled, we are only here to declare our options (there are
  # none, currently), so leave now
  return()
endif()

# END user configurable build options

###############################################################################

# BEGIN core libraries and includes

if(QtTesting_FOUND AND BUILD_TESTING)
  include_directories(${qtTestingSupport_SOURCE_DIR})
  add_definitions(-DENABLE_QTTESTING)
  set(QT_TESTING_SUPPORT_LIBRARIES qtTestingSupport ${QtTesting_LIBRARIES})
endif()

# Use the include path and library for Qt that is used by VTK

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QtTesting_INCLUDE_DIRS}
)

set(vspSdkTargets
  vspSourceUtil
  vspUserInterface # Inherit the rest of our dependencies from this
)

# Configure version header here
include(vsVersion.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vsVersion.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/vsVersion.h @ONLY)

# END core libraries and includes

###############################################################################

# BEGIN built-in plugins

set(vspPluginSources
    # Archive sources
    Plugins/vsArchiveSourceFactory.cxx
    Plugins/vsArchiveSourcePlugin.cxx
    Plugins/vsVideoArchiveSourcePlugin.cxx
    Plugins/vsTrackArchiveSourcePlugin.cxx
    Plugins/vsDescriptorArchiveSourcePlugin.cxx
    # Other sources
    Plugins/vsVvqsDatabaseFactory.cxx
    Plugins/vsVvqsDatabaseQueryDialog.cxx
    Plugins/vsVvqsDatabaseSource.cxx
    Plugins/vsVvqsDatabaseSourcePlugin.cxx
)

# END built-in plugins

###############################################################################

# BEGIN program code

set(vspSources
    # UI library wrapper
    main.cxx
    # Built-in source plugins
    ${vspPluginSources}
)

set(vspResSources
  ../../Icons/stdbutton.qrc
  ../../Icons/vgvideo.qrc
  ../../Icons/vsp.qrc
)

# END core program code

###############################################################################

# BEGIN build rules

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
  ${vspSources} ${vspResSources}
)

# Built-in plugins should be considered static plugins
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_STATICPLUGIN)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ${vspSdkTargets}
  ${QT_TESTING_SUPPORT_LIBRARIES}
  Qt5::Network
)

vg_add_test_subdirectory()

install_executable_target(${PROJECT_NAME} ${PROJECT_NAME})

# END build rules
