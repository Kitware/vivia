project(vvVtkWidgets)

###############################################################################

# BEGIN common sources and includes

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vvVtkWidgetsMocHeaders
  vvAbstractSimilarityQueryDialog.h
  vvClipVideoRepresentation.h
  vvGenerateReportDialog.h
  vvQueryVideoPlayer.h
  vvReportWriter.h
  vvVideoPlayer.h
  vvVideoPlayerModel.h
  vvVideoQueryDialog.h
)

set(vvVtkWidgetsSources
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

set(vvVtkWidgetsUiFiles
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

#END common sources

###############################################################################

# BEGIN OPTIONAL kwppt generator

if (${VISGUI_ENABLE_KWPPT})
  find_package(KWPowerPointGenerator REQUIRED)

  set(KWPPT_LIBRARY KWPowerPointWrapper)

  list(APPEND vvVtkWidgetsSources
    vvGeneratePowerPointDialog.cxx
    vvPowerPointEventSlideGenerator.cxx
    vvPowerPointSlideGenerator.cxx
    vvPowerPointTrackSlideGenerator.cxx
    vvPowerPointWriter.cxx
  )
  list(APPEND vvVtkWidgetsMocHeaders
    vvGeneratePowerPointDialog.h
    vvPowerPointWriter.h
  )
  list(APPEND vvVtkWidgetsInstallHeaders
    vvGeneratePowerPointDialog.h
    vvPowerPointEventSlideGenerator.h
    vvPowerPointExtensionInterface.h
    vvPowerPointSlideGenerator.h
    vvPowerPointTrackSlideGenerator.h
    vvPowerPointWriter.h
  )
  list(APPEND vvVtkWidgetsUiFiles
    generatePowerPoint.ui
  )
endif()

#END OPTIONAL kwppt generator

###############################################################################

# BEGIN build rules

qt4_wrap_cpp(mocSources ${vvVtkWidgetsMocHeaders})
qt4_wrap_ui(uiSources ${vvVtkWidgetsUiFiles})

add_library(${PROJECT_NAME}
  ${vvVtkWidgetsSources}
  ${mocSources}
  ${uiSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vvWidgets
  qtVgWidgets
  qtVgCommon
  qtExtensions
  vgVtkVideo
  vtkVgQtWidgets
  PRIVATE_INTERFACE_TARGETS
  vtkVgModelView
  vtkVgSceneGraph
  vtkVgQtUtil
  LINK_LIBRARIES
  LINK_PRIVATE
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkCommonSystem
  vtkGUISupportQt
  ${KWPPT_LIBRARY}
  LINK_PUBLIC
  ${QT_LIBRARIES}
)

install_library_targets(${PROJECT_NAME})
install_headers(${vvVtkWidgetsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvVtkWidgets)

# END build rules
