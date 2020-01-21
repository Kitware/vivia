project(vsPowerPointGeneratorExtension)

include_directories(SYSTEM
  ${KML_INCLUDE_DIRS}
)

set(vsPowerPointGeneratorExtension_Sources
  vsPowerPointGenerator.cxx
  vsPowerPointGeneratorInterface.cxx
  vsPowerPointGeneratorPlugin.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsPowerPointGeneratorExtension_Sources})

target_link_libraries(${PROJECT_NAME}
  vgVtkVideo
  vspData
  vspUserInterface
)

install_plugin_targets(${PROJECT_NAME})
