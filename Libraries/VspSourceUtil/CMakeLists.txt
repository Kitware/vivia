project(vspSourceUtil)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(vspSourceUtil_Sources
    vsAdapt.cxx
    vsArchiveSource.cxx
    vsSourceFactoryPlugin.cxx
    vsStreamFactory.cxx
    vsStreamSource.cxx
    vsUtilDebug.cxx
    vsVideoArchive.cxx
    vsVideoHelper.cxx
)

set(vspSourceUtilInstallHeaders
  vsAdapt.h
  vsAdaptTracks.h
  vsArchiveSource.h
  vsArchiveSourcePrivate.h
  vsSourceFactoryPlugin.h
  vsStreamFactory.h
  vsStreamSource.h
  vsStreamSourcePrivate.h
  vsVideoArchive.h
  vsVideoHelper.h
)

add_library(${PROJECT_NAME} ${vspSourceUtil_Sources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vspData
)

install_library_targets(${PROJECT_NAME})
install_headers(${vspSourceUtilInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VspSourceUtil)
