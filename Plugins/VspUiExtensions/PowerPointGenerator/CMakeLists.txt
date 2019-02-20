project(vsPowerPointGeneratorExtension)

include_directories(SYSTEM
  ${KML_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Need this as a directory include so that moc can find the plugin interface
vg_include_library_sdk_directories(vspUserInterface)

set(vsPowerPointGeneratorExtension_Sources
  vsPowerPointGenerator.cxx
  vsPowerPointGeneratorInterface.cxx
  vsPowerPointGeneratorPlugin.cxx
)

set(vsPowerPointGeneratorExtension_MocHeaders
  vsPowerPointGeneratorInterface.h
  vsPowerPointGeneratorPlugin.h
)

qt5_wrap_cpp(vsPowerPointGeneratorExtension_MocSources
             ${vsPowerPointGeneratorExtension_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsPowerPointGeneratorExtension_Sources}
  ${vsPowerPointGeneratorExtension_MocSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  vgVtkVideo
  vspData
  vspUserInterface
  LINK_LIBRARIES
  LINK_PRIVATE
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
