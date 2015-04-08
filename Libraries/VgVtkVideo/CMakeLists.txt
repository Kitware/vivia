project(vgVtkVideo)

set(vvvSources
  vgVideoAnimation.cxx
  vgVideoSeekRequest.cxx
  vgVideoPlayer.cxx
  vgVideoPlayerBufferedRequestor.cxx
  vgVideoPlayerRequestor.cxx
  vgVideoRequestor.cxx
)

set(vvvMocHeaders
  vgVideoAnimation.h
  vgVideoPlayer.h
  vgVideoPlayerBufferedRequestor.h
  vgVideoPlayerPrivate.h
  vgVideoPlayerRequestor.h
  vgVideoSourceRequestor.h
)

set(vgVtkVideoInstallHeaders
  vgVideoAnimation.h
  vgVideoPlayer.h
  vgVideoRequestor.h
  vgVideoSource.h
  vgVideoSourceRequestor.h
  vgVtkVideoFrame.h
)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

qt4_wrap_cpp(mocSources ${vvvMocHeaders})

source_group("Generated" FILES ${mocSources})

add_library(${PROJECT_NAME} ${vvvSources} ${mocSources})

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  qtExtensions
  vgVideo
  vtkVgCore
  vtkVgVideo
  LINK_LIBRARIES
  LINK_PUBLIC
  ${QT_LIBRARIES}
  vtkRenderingCore
  ${VTK_JPEG_LIBRARIES}
  LINK_PRIVATE
  vgl
  vgl_algo
)

install_library_targets(${PROJECT_NAME})
install_headers(${vgVtkVideoInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VgVtkVideo)
