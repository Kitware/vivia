project(vsRandomAlertDescriptor)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
vg_include_library_sdk_directories(vspSourceUtil)

set(vsRandomAlertDescriptor_Sources
  vsRandomAlertDescriptor.cxx
  vsRandomAlertDescriptorPlugin.cxx
  vsRandomAlertFactory.cxx
)

set(vsRandomAlertDescriptor_MocHeaders
  vsRandomAlertDescriptor.h
  vsRandomAlertDescriptorPlugin.h
)

qt5_wrap_cpp(vsRandomAlertDescriptor_MocSources
             ${vsRandomAlertDescriptor_MocHeaders})

vg_add_qt_plugin(${PROJECT_NAME}
  ${vsRandomAlertDescriptor_Sources}
  ${vsRandomAlertDescriptor_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
