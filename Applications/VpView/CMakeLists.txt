project(vpView)

###############################################################################

# BEGIN user configurable build options

vg_option(VPVIEW_ASSIGN_FAKE_TRACK_ATTRIBUTES OFF
  "Assign fake track state attributes ***FOR TESTING ONLY***"
  "VISGUI_ENABLE_VPVIEW;VISGUI_ENABLE_VIDTK"
)
mark_as_advanced(VPVIEW_ASSIGN_FAKE_TRACK_ATTRIBUTES)

if(NOT VISGUI_ENABLE_VPVIEW)
  # If vpView is disabled, we are only here to declare our options, so leave
  # now
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

if(VPVIEW_ASSIGN_FAKE_TRACK_ATTRIBUTES)
  add_definitions(-DVPVIEW_ASSIGN_FAKE_TRACK_ATTRIBUTES)
endif()

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${LIBJSON_INCLUDE_DIR}
  ${VTK_INCLUDE_DIRS}
  ${GeographicLib_INCLUDE_DIRS}
)

if(VISGUI_ENABLE_VIDTK)
  include_directories(SYSTEM
    ${VIDTK_INCLUDE_DIRS}
    ${VIDTK_INCLUDE_DIRS}/..
  )
endif()

if(VISGUI_ENABLE_KWIVER)
   include_directories(SYSTEM
     ${KWIVER_INCLUDE_DIRS}
   )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QtTesting_INCLUDE_DIRS}
)

set(vgSdkTargets
  vgCommon
  vgVtkVideo
  qtExtensions
  qtVgCommon
  qtVgWidgets
  vtkVgCore
  vtkVgModelView
  vtkVgQtUtil
  vgDataFramework
)

# If this is true, boost is required
if(VIDTK_IS_UUIDABLE)
  link_directories(${Boost_LIBRARY_DIRS})
endif()

# Needed for GDAL readers
if(VISGUI_ENABLE_GDAL)
  list(APPEND vgSdkTargets vtkVgIO)
endif()

vg_include_library_sdk_directories(${vgSdkTargets} vvData)

# Configure version file
include(vpVersion.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vpVersion.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vpVersion.h @ONLY)

# END core libraries and includes

###############################################################################

# BEGIN core program code

set(vpViewSources
  main.cxx
  vpActivityConfig.cxx
  vpActivityIO.cxx
  vpAnnotation.cxx
  vpAttributeConfig.cxx
  vpBox.cxx
  vpConfigUtils.cxx
  vpConfigureDialog.cxx
  vpContour.cxx
  vpCreateEventDialog.cxx
  vpEditTrackTypesDialog.cxx
  vpEntityConfigWidget.cxx
  vpEventConfig.cxx
  vpEventIO.cxx
  vpEventTypeRegistry.cxx
  vpExternalProcessDialog.cxx
  vpFileDataSource.cxx
  vpFileEventIO.cxx
  vpFileEventIOImpl.cxx
  vpFileTrackIO.cxx
  vpFileTrackIOImpl.cxx
  vpFilterTypeDelegate.cxx
  vpGraphModelHelper.cxx
  vpGraphModelView.cxx
  vpFseTrackIO.cxx
  vpGraphModelWidget.cxx
  vpFrameMap.cxx
  vpFseIO.cxx
  vpImageSourceFactory.cxx
  vpInformaticsDialog.cxx
  vpMergeTracksDialog.cxx
  vpModelIO.cxx
  vpMultiGraphModel.cxx
  vpMultiGraphRepresentation.cxx
  vpNormalcyMaps.cxx
  vpObjectInfoPanel.cxx
  vpObjectSelectionPanel.cxx
  vpPrimitiveConfig.cxx
  vpProject.cxx
  vpProjectList.cxx
  vpProjectParser.cxx
  vpQtSceneUtils.cxx
  vpQtViewer3d.cxx
  vpQtViewer3dDialog.cxx
  vpQtViewer3dWidget.cxx
  vpSelectTimeIntervalDialog.cxx
  vpSessionView.cxx
  vpSettings.cxx
  vpTimelineDialog.cxx
  vpTrackColorDialog.cxx
  vpTrackConfig.cxx
  vpTrackIO.cxx
  vpTreeView.cxx
  vpUtils.cxx
  vpVideoAnimation.cxx
  vpView.cxx
  vpViewCore.cxx
  vtkVpTrackModel.cxx
)

set(vpViewUI
  qtInformaticsDialog.ui
  qtNormalcyDialog.ui
  qtRenderWindowDialog.ui
  vpConfigure.ui
  vpCreateEventDialog.ui
  vpEditTrackTypesDialog.ui
  vpExternalProcessDialog.ui
  vpGraphModelWidget.ui
  vpMergeTracksDialog.ui
  vpObjectInfoPanel.ui
  vpObjectSelectionPanel.ui
  vpQtViewer3dDialog.ui
  vpQtViewer3dWidget.ui
  vpSelectTimeIntervalDialog.ui
  vpTimelineDialog.ui
  vpTrackAttributeColorDialog.ui
  vpTrackColorDialog.ui
  vpView.ui
)

set(vpViewHeaders
  vpApplication.h
  vpConfigureDialog.h
  vpCreateEventDialog.h
  vpEditTrackTypesDialog.h
  vpEntityConfigWidget.h
  vpExternalProcessDialog.h
  vpFileDataSource.h
  vpGraphModelHelper.h
  vpGraphModelView.h
  vpGraphModelWidget.h
  vpFrameMap.h
  vpInformaticsDialog.h
  vpMergeTracksDialog.h
  vpObjectInfoPanel.h
  vpObjectSelectionPanel.h
  vpProjectList.h
  vpQtViewer3d.h
  vpQtViewer3dDialog.h
  vpQtViewer3dWidget.h
  vpSelectTimeIntervalDialog.h
  vpSessionView.h
  vpTimelineDialog.h
  vpTabWidget.h
  vpTrackColorDialog.h
  vpTreeView.h
  vpVideoAnimation.h
  vpView.h
  vpViewCore.h
  vtkVpTrackModel.h
)

set(vpViewResources ../../Icons/vpView.qrc)

# END core program code

###############################################################################

# BEGIN OPTIONAL vidtk io

if(VISGUI_ENABLE_VIDTK)
  list(APPEND vpViewSources
    vpVidtkActivityIO.cxx
    vpVidtkEventIO.cxx
    vpVidtkFileEventIO.cxx
    vpVidtkFileIO.cxx
    vpVidtkFileReader.cxx
    vpVidtkFileTrackIO.cxx
    vpVidtkIO.cxx
    vpVidtkTrackIO.cxx
  )
endif()

# END OPTIONAL vidtk io

###############################################################################

# BEGIN OPTIONAL kwiver integration

if(VISGUI_ENABLE_KWIVER)
  list(APPEND vpViewSources
    vpKwiverImproveTrackWorker.cxx
    vpKwiverVideoSource.cxx
  )

  list(APPEND vpViewHeaders
    vpKwiverImproveTrackWorker.h
  )
endif()

# END OPTIONAL kwiver integration

###############################################################################

# BEGIN build rules

qt4_wrap_ui(uiSources ${vpViewUI})
qt4_wrap_cpp(mocSources ${vpViewHeaders})
qt4_add_resources(resSources ${vpViewResources})

source_group("Resources" FILES
  ${vpViewUI}
  ${EXE_ICON}
)

source_group("Generated" FILES
  ${uiSources}
  ${mocSources}
  ${ResourceSources}
)

set_source_files_properties(${vpViewSources}
  PROPERTIES OBJECT_DEPENDS "${uiSources}"
)

# Hide the console window on release builds
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(ExecProperties WIN32)
endif()

set(ExecProperties ${ExecProperties} MACOSX_BUNDLE)

add_executable(${PROJECT_NAME} ${ExecProperties}
  ${vpViewSources} ${uiSources} ${mocSources} ${resSources}
)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
  ${vgSdkTargets}
  vgl vnl vul vil_io
  ${GeographicLib_LIBRARIES}
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkChartsCore vtkCommonColor vtkCommonSystem vtkInfovisCore
  vtkRenderingAnnotation vtkRenderingCore vtkGUISupportQt
  vtkViewsInfovis vtkViewsContext2D
  ${LIBJSON_LIBRARY}
  ${QT_TESTING_SUPPORT_LIBRARIES}
  ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES}
)

if(VISGUI_ENABLE_KWIVER)
  target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
    kwiver::kwiver_algo_vxl
    kwiver::kwiver_algo_core
  )
endif()

if(VISGUI_ENABLE_VIDTK)
  target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
    vidtk_activity_detectors_lib
    vidtk_events_lib
    vidtk_tracking
    vidtk_utilities
  )
endif()

vg_add_test_subdirectory()

install_executable_target(${PROJECT_NAME} ${PROJECT_NAME})

# END build rules
