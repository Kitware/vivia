project(vsRandomAlertDescriptor)

set(vsRandomAlertDescriptor_Sources
  vsRandomAlertDescriptor.cxx
  vsRandomAlertDescriptorPlugin.cxx
  vsRandomAlertFactory.cxx
)

vg_add_qt_plugin(${PROJECT_NAME} ${vsRandomAlertDescriptor_Sources})

target_link_libraries(${PROJECT_NAME}
  vspSourceUtil
)

install_plugin_targets(${PROJECT_NAME})
