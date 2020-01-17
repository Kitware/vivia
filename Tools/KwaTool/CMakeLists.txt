project(kwaTool)

set(SRCS kwaTool.cxx)

add_executable(${PROJECT_NAME} ${SRCS})

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  vgVideo
  qtExtensions
  LINK_LIBRARIES LINK_PRIVATE
  ${QT_LIBRARIES}
)

install_executable_target(${PROJECT_NAME} Tools)
