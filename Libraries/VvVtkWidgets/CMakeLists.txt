project(vvVtkWidgets)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vgVtkSharedMocHeaders
  vvAbstractSimilarityQueryDialog.h
  vvClipVideoRepresentation.h
  vvGenerateReportDialog.h
  vvQueryVideoPlayer.h
  vvReportWriter.h
  vvVideoPlayer.h
  vvVideoPlayerModel.h
  vvVideoQueryDialog.h
)

set(vgVtkSharedSources
  vvAbstractSimilarityQueryDialog.cxx
  vvClipVideoRepresentation.cxx
  vvEventInfo.cxx
  vvGenerateReportDialog.cxx
  vvQueryVideoPlayer.cxx
  vvReportWriter.cxx
  vvTrackInfo.cxx
  vvVideoPlayer.cxx
  vvVideoPlayerModel.cxx
  vvVideoQueryDialog.cxx
  vvVideoQueryDialogPrivate.cxx
)

set(vgVtkSharedUiFiles
  generateReport.ui
  videoQuery.ui
  videoView.ui
)

set(vvVtkWidgetsInstallHeaders
  vvAbstractSimilarityQueryDialog.h
  vvClipVideoRepresentation.h
  vvEventInfo.h
  vvGenerateReportDialog.h
  vvQueryVideoPlayer.h
  vvQueryVideoPlayerPrivate.h
  vvReportWriter.h
  vvTrackInfo.h
  vvVideoPlayer.h
  vvVideoPlayerModel.h
  vvVideoPlayerPrivate.h
  vvVideoQueryDialog.h
  vvVideoQueryDialogPrivate.h
  ${CMAKE_CURRENT_BINARY_DIR}/ui_videoQuery.h
  ${CMAKE_CURRENT_BINARY_DIR}/ui_videoView.h
)

qt4_wrap_cpp(mocSources ${vgVtkSharedMocHeaders})
qt4_wrap_ui(uiSources ${vgVtkSharedUiFiles})

add_library(${PROJECT_NAME}
  ${vgVtkSharedSources}
  ${mocSources}
  ${uiSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vvWidgets
  qtVgWidgets
  qtVgCommon
  qtExtensions
  vtkVgQtWidgets
  PRIVATE_INTERFACE_TARGETS
  vtkVgModelView
  vtkVgSceneGraph
  vtkVgQtUtil
  LINK_LIBRARIES
  LINK_PRIVATE
  vtkGUISupportQt
  vtkRenderingFreeTypeOpenGL
  LINK_PUBLIC
  ${QT_LIBRARIES}
)

install_library_targets(${PROJECT_NAME})
install_headers(${vvVtkWidgetsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvVtkWidgets)
