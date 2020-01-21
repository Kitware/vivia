project(qsettings-tool)

add_executable(${PROJECT_NAME} qsettingsTool.cxx)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  qtExtensions
  qtVgCommon
)

install_executable_target(${PROJECT_NAME} Tools)
