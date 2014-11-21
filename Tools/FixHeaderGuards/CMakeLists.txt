project(fixHeaderGuards)

add_executable(${PROJECT_NAME} fixHeaderGuards.cxx)

target_link_libraries(${PROJECT_NAME}
  ${QT_LIBRARIES}
)
