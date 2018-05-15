project(qsettings-tool)

add_executable(${PROJECT_NAME} qsettingsTool.cxx)

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  qtExtensions
  qtVgCommon
  LINK_LIBRARIES LINK_PRIVATE
  ${QT_LIBRARIES}
)

install_executable_target(${PROJECT_NAME} Tools)
