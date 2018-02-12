project(viqui)

###############################################################################

# BEGIN user configurable build options

if(NOT VISGUI_ENABLE_VIQUI)
  # If viqui is disabled, we are only here to declare our options (there are
  # none, currently), so leave now
  return()
endif()

# END user configurable build options

###############################################################################

# BEGIN core libraries and includes

if(QtTesting_FOUND AND BUILD_TESTING)
  include_directories(${qtTestingSupport_SOURCE_DIR})
  add_definitions(-DENABLE_QTTESTING)
  set(QT_TESTING_SUPPORT_LIBRARIES qtTestingSupport ${QtTesting_LIBRARIES})
endif()

# Use the include path and library for Qt that is used by VTK.
include_directories(SYSTEM
  ${KML_INCLUDE_DIRS}
  ${QtTesting_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${GeographicLib_INCLUDE_DIRS}
  ${PROJ4_INCLUDE_DIR}
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(vgSdkTargets
  vvVtkWidgets
  qtExtensions
  qtVgCommon
  qtVgWidgets
  vgVtkVideo
  vtkVgCore
  vtkVgSceneGraph
  vtkVgModelView
  vtkVgQtSceneUtil
  vtkVgQtUtil
  vgVideo
  vvIO
  vvWidgets
)

vg_include_library_sdk_directories(${vgSdkTargets})

# Configure file here
include(vqVersion.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vqVersion.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vqVersion.h @ONLY)

# END core libraries and includes

###############################################################################

# BEGIN core program code

set(vqSources
    main.cxx
    vqApplication.cxx
    vqArchiveVideoSource.cxx
    vqClassifierQueryDialog.cxx
    vqConfigureDialog.cxx
    vqContour.cxx
    vqCore.cxx
    vqDebug.cxx
    vqEventInfo.cxx
    vqPredefinedQueryCache.cxx
    vqPredefinedQueryDialog.cxx
    vqQueryDialog.cxx
    vqQueryParser.cxx
    vqQueryVideoPlayer.cxx
    vqRegionEditDialog.cxx
    vqResultFilter.cxx
    vqResultFilterDialog.cxx
    vqResultInfo.cxx
    vqSettings.cxx
    vqTimeline.cxx
    vqTrackingClipViewer.cxx
    vqTreeView.cxx
    vqUserActions.cxx
    vqUtil.cxx
    vqVideoPlayer.cxx
    vqVideoQueryDialog.cxx
    vtkVQBlastLayoutNode.cxx
    vtkVQTrackingClip.cxx
    Backends/vqExporterFactory.cxx
    Backends/vqVqrExporter.cxx
)

set(vqUI
    classifierQuery.ui
    configure.ui
    predefinedQuery.ui
    query.ui
    regionEdit.ui
    resultFilter.ui
    resultInfo.ui
    viqui.ui
)

set(vqMocHeaders
    vqApplication.h
    vqArchiveVideoSource.h
    vqClassifierQueryDialog.h
    vqConfigureDialog.h
    vqCore.h
    vqPredefinedQueryCache.h
    vqPredefinedQueryCachePrivate.h
    vqPredefinedQueryDialog.h
    vqQueryDialog.h
    vqQueryParser.h
    vqQueryVideoPlayer.h
    vqRegionEditDialog.h
    vqResultFilterDialog.h
    vqResultInfo.h
    vqTimeline.h
    vqTrackingClipBuilder.h
    vqTrackingClipViewer.h
    vqTreeView.h
    vqUserActions.h
    vqVideoPlayer.h
    vqVideoQueryDialog.h
    vtkVQBlastLayoutNode.h
)

set(vqResources ../../Icons/stdbutton.qrc ../../Icons/viqui.qrc ../../Icons/vgvideo.qrc)

# END core program code

###############################################################################

# BEGIN build rules

qt4_wrap_ui(uiSources ${vqUI})
qt4_wrap_cpp(mocSources ${vqMocHeaders})
qt4_add_resources(resSources ${vqResources})

source_group("User Interface" FILES
  ${vqUI}
)

source_group("Resources" FILES
  ${vqUI}
  ${EXE_ICON}
)

source_group("Generated" FILES
  ${uiSources}
  ${mocSources}
  ${resSources}
)

set_source_files_properties(${vqSources}
  PROPERTIES OBJECT_DEPENDS "${uiSources}"
)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
  ${vqSources} ${uiSources} ${mocSources} ${resSources}
)

target_link_libraries(${PROJECT_NAME}
  # Internal libraries
  ${vgSdkTargets}
  # OPTIONAL external libraries
  ${QT_TESTING_SUPPORT_LIBRARIES}
  # Required external libraries
  vtkChartsCore
  vtkCommonSystem
  vtkGUISupportQt
  vtkRenderingCore
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkViewsContext2D
  vil_io
  vnl_io
  vgl_algo
  ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES}
)

install_executable_target(${PROJECT_NAME} ${PROJECT_NAME})

# END build rules
