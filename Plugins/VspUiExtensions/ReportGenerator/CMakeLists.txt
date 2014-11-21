project(vsReportGeneratorExtension)

include_directories(SYSTEM
  ${KML_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

vg_include_library_sdk_directories(vspUserInterface)

set(vsReportGeneratorExtension_Sources
  vsKmlWriter.cxx
  vsReportGenerator.cxx
  vsReportGeneratorInterface.cxx
  vsReportGeneratorPlugin.cxx
  vsVideoRequestor.cxx
)

set(vsReportGeneratorExtension_MocHeaders
  vsReportGeneratorInterface.h
  vsReportGeneratorPlugin.h
)

qt4_wrap_cpp(vsReportGeneratorExtension_MocSources
             ${vsReportGeneratorExtension_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsReportGeneratorExtension_Sources}
  ${vsReportGeneratorExtension_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vspUserInterface
  ${QT_LIBRARIES}
)

install_plugin_targets(${PROJECT_NAME})
