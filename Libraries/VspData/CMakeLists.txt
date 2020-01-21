project(vspData)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(vspData_Sources
    # Service core
    vsSourceFactoryInterface.cxx
    vsSourceService.cxx
    # Data sources
    vsDataSource.cxx
    vsDescriptorSource.cxx
    vsTrackSource.cxx
    vsVideoSource.cxx
    vsSimpleSourceFactory.cxx
    vsSourceFactory.cxx
    vsStaticSourceFactory.cxx
    # Live descriptors
    vsLiveDescriptor.cxx
    # Supporting code
    vsContour.cxx
    vsDescriptorInput.cxx
    vsEventInfo.cxx
    vsLibDebug.cxx
    vsTrackInfo.cxx
    vtkVsTrackInfo.cxx
)

set(vspDataInstallHeaders
  vsArchiveSourceInfo.h
  vsArchiveSourceInterface.h
  vsContour.h
  vsDataSource.h
  vsDescriptor.h
  vsDescriptorInput.h
  vsDescriptorSource.h
  vsDisplayInfo.h
  vsEvent.h
  vsEventInfo.h
  vsFactoryAction.h
  vsLiveDescriptor.h
  vsLiveDescriptorPrivate.h
  vsSimpleSourceFactory.h
  vsSourceFactory.h
  vsSourceFactoryInterface.h
  vsSourceService.h
  vsStaticSourceFactory.h
  vsTrackClassifier.h
  vsTrackData.h
  vsTrackId.h
  vsTrackInfo.h
  vsTrackSource.h
  vsTrackState.h
  vsVideoSource.h
  vsVideoSourcePrivate.h
  vtkVsTrackInfo.h
)

add_library(${PROJECT_NAME} ${vspData_Sources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  qtExtensions
  qtVgCommon
  vgVideo
  vtkVgCore
  vvIO
  vgVtkVideo
  vvVtkWidgets
)

install_library_targets(${PROJECT_NAME})
install_headers(${vspDataInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VspData)

vg_add_documentation(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}
  Qt
  qtExtensions
)
