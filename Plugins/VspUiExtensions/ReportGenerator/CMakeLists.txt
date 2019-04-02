project(vsReportGeneratorExtension)

include_directories(SYSTEM
  ${KML_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Need this as a directory include so that moc can find the plugin interface
vg_include_library_sdk_directories(vspUserInterface)

if(VSPUI_ENABLE_REPORT_GENERATOR)
  add_definitions(-DENABLE_REPORT_GENERATION)
endif()
if(VSPUI_ENABLE_KML_EXPORT)
  add_definitions(-DENABLE_KML_EXPORT)
endif()

set(vsReportGeneratorExtension_Sources
  vsKmlWriter.cxx
  vsReportGeneratorInterface.cxx
  vsReportGeneratorPlugin.cxx
)

if(VSPUI_ENABLE_REPORT_GENERATOR)
  list(APPEND vsReportGeneratorExtension_Sources
    vsReportGenerator.cxx
  )
endif()

set(vsReportGeneratorExtension_MocHeaders
  vsReportGeneratorInterface.h
  vsReportGeneratorPlugin.h
)

qt5_wrap_cpp(vsReportGeneratorExtension_MocSources
             ${vsReportGeneratorExtension_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsReportGeneratorExtension_Sources}
  ${vsReportGeneratorExtension_MocSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  vspData
  vspUserInterface
)

install_plugin_targets(${PROJECT_NAME})
