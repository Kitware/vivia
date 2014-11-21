vg_include_library_sdk_directories(
  TARGET vvData INTERFACE vgCommon
)

set(vvData_InstallHeaders
  vvDescriptor.h
  vvIqr.h
  vvQuery.h
  vvQueryResult.h
  vvTrack.h
  vvUserData.h
)

install_headers(${vvData_InstallHeaders} TARGET vvData
                DESTINATION include/VvData)
