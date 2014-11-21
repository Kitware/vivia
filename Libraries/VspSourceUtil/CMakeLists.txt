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

set(vspSourceUtil_MocHeaders
    vsStreamSource.h
    vsStreamSourcePrivate.h
    vsVideoArchive.h
    vsVideoHelper.h
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

qt4_wrap_cpp(vspSourceUtil_MocSources ${vspSourceUtil_MocHeaders})

add_library(${PROJECT_NAME}
  ${vspSourceUtil_Sources}
  ${vspSourceUtil_MocSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vspData
)

install_library_targets(${PROJECT_NAME})
install_headers(${vspSourceUtilInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VspSourceUtil)
