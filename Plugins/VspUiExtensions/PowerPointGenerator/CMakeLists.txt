project(vsPowerPointGeneratorExtension)

include_directories(SYSTEM
  ${KML_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

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

target_link_libraries(${PROJECT_NAME}
  vgVtkVideo
  vspData
  vspUserInterface
)

install_plugin_targets(${PROJECT_NAME})
