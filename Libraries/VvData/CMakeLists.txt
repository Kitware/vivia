project(vvData)

set(vvData_InstallHeaders
  vvDescriptor.h
  vvEvent.h
  vvIqr.h
  vvQuery.h
  vvQueryResult.h
  vvTrack.h
  vvUserData.h
)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME}
  INTERFACE
  vgCommonHeaders
)

install_library_targets(${PROJECT_NAME})
install_headers(${vvData_InstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvData)
