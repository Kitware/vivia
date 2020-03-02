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

get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_is_multi_config)
  set(${PROJECT_NAME}_AUTOGEN_INCLUDE_DIR
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/include_$<CONFIG>)
else()
  set(${PROJECT_NAME}_AUTOGEN_INCLUDE_DIR
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/include)
endif()

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
  ${${PROJECT_NAME}_AUTOGEN_INCLUDE_DIR}/ui_videoQuery.h
  ${${PROJECT_NAME}_AUTOGEN_INCLUDE_DIR}/ui_videoView.h
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

 list(APPEND vvVtkWidgetsInstallHeaders
    vvGeneratePowerPointDialog.h
    vvPowerPointEventSlideGenerator.h
    vvPowerPointExtensionInterface.h
    vvPowerPointSlideGenerator.h
    vvPowerPointTrackSlideGenerator.h
    vvPowerPointWriter.h
  )
endif()

#END OPTIONAL kwppt generator

###############################################################################

# BEGIN build rules

add_library(${PROJECT_NAME} ${vvVtkWidgetsSources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vvWidgets
  qtVgWidgets
  qtVgCommon
  qtExtensions
  vgVtkVideo
  vtkVgQtWidgets
  PRIVATE
  vtkVgModelView
  vtkVgSceneGraph
  vtkVgQtUtil
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkCommonSystem
  vtkGUISupportQt
  ${KWPPT_LIBRARY}
)

install_library_targets(${PROJECT_NAME})
install_headers(${vvVtkWidgetsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvVtkWidgets)

# END build rules
