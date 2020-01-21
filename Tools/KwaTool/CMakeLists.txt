project(kwaTool)

set(SRCS kwaTool.cxx)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  vgVideo
  qtExtensions
)

install_executable_target(${PROJECT_NAME} Tools)
