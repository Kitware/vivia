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

vg_include_library_sdk_directories(${vspSdkTargets})

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

set(vspPluginMocHeaders
    # Archive sources
    Plugins/vsVideoArchiveSourcePlugin.h
    Plugins/vsTrackArchiveSourcePlugin.h
    Plugins/vsDescriptorArchiveSourcePlugin.h
    # Other sources
    Plugins/vsVvqsDatabaseQueryDialog.h
    Plugins/vsVvqsDatabaseSource.h
    Plugins/vsVvqsDatabaseSourcePrivate.h
    Plugins/vsVvqsDatabaseSourcePlugin.h
)

set(vspPluginUI
    Plugins/vvqsDatabaseQuery.ui
)

# Built-in plugins should be considered static plugins
add_source_files_definitions(${vspPluginSources} SYMBOLS QT_STATICPLUGIN)

# END built-in plugins

###############################################################################

# BEGIN program code

set(vspSources
    # UI library wrapper
    main.cxx
    # Built-in source plugins
    ${vspPluginSources}
)

set(vspUI
    ${vspPluginUI}
)

set(vspMocHeaders
    ${vspPluginMocHeaders}
)

set(vspResources
  ../../Icons/stdbutton.qrc
  ../../Icons/vgvideo.qrc
  ../../Icons/vsp.qrc
)

# END core program code

###############################################################################

# BEGIN build rules

qt5_wrap_ui(uiSources ${vspUI})
qt5_wrap_cpp(mocSources ${vspMocHeaders})
qt5_add_resources(resSources ${vspResources})

source_group("User Interface" FILES
  ${vspUI}
)

source_group("Resources" FILES
  ${vspUI}
  ${EXE_ICON}
)

source_group("Generated" FILES
  ${uiSources}
  ${mocSources}
  ${resSources}
)

set_source_files_properties(${vspSources}
  PROPERTIES OBJECT_DEPENDS "${uiSources}"
)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
  ${vspSources} ${uiSources} ${mocSources} ${resSources}
)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
  ${vspSdkTargets}
  ${QT_TESTING_SUPPORT_LIBRARIES}
)

vg_add_test_subdirectory()

install_executable_target(${PROJECT_NAME} ${PROJECT_NAME})

# END build rules
