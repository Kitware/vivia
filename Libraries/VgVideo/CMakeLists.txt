project(vgVideo)

vg_include_library_sdk_directories(
  TARGET ${PROJECT_NAME} INTERFACE
  vgCommon
  qtVgCommon
)

set(vgVideoSources
    vgImage.cxx
    vgIStream.cxx
    vgKwaArchive.cxx
    vgKwaFrameMetadata.cxx
    vgKwaUtil.cxx
    vgKwaVideoClip.cxx
    vgVideo.cxx
    vgVideoBuffer.cxx
    vgVideoFramePtr.cxx
    vslInstantiations.cxx
)

set(vgVideoInstallHeaders
  vgImage.h
  vgKwaArchive.h
  vgKwaFrameMetadata.h
  vgKwaVideoClip.h
  vgVideoBuffer.h
  vgVideoFramePtr.h
  vgVideo.h
)

set(vgVideoWrapObjects
  vgImage
  vgKwaArchive
  vgKwaArchiveRequest
  vgKwaFrameMetadata
  vgKwaVideoClip
  vgVideo
  vgVideoBuffer
  BY_VALUE
  vgKwaWorldBox
  vgVideoFramePtr
)

add_library(${PROJECT_NAME} ${vgVideoSources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vgCommonHeaders
  qtExtensionsHeaders
  Qt5::Gui
  PRIVATE
  qtExtensions
  vnl_io
  vil_io
  vsl
)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vgVideoInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VgVideo)

vg_wrap_library(${PROJECT_NAME}
  OBJECTS ${vgVideoWrapObjects}
  HEADERS ${vgVideoInstallHeaders}
  DEPENDS PySide:Core vgCommon
)
