project(vspUserInterface)

include(${qtExtensions_SOURCE_DIR}/CMake/Modules/UseQtExtensions.cmake)

# Use the include path and library for Qt that is used by VTK
include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(QtTesting_FOUND AND BUILD_TESTING)
  add_definitions(-DENABLE_QTTESTING)
  include_directories(SYSTEM ${QtTesting_INCLUDE_DIRS})
  set(TESTING_INTERFACES qtTestingSupport)
endif()

###############################################################################

# BEGIN source file lists

set(vspUserInterface_Sources
    # Core code
    vsApplication.cxx
    vsCore.cxx
    vsCorePrivate.cxx
    vsMainWindow.cxx
    vsMainWindowPrivate.cxx
    vsScene.cxx
    vsScenePrivate.cxx
    # Supporting code
    vsAlertEditor.cxx
    vsAlertList.cxx
    vsContourWidget.cxx
    vsDebug.cxx
    vsEventDataModel.cxx
    vsEventInfoWidget.cxx
    vsEventRatingMenu.cxx
    vsEventTreeModel.cxx
    vsEventTreeSelectionModel.cxx
    vsEventTreeView.cxx
    vsEventTreeWidget.cxx
    vsNoteTreeWidget.cxx
    vsRegionList.cxx
    vsRegionTypeDelegate.cxx
    vsSettings.cxx
    vsTrackColorDialog.cxx
    vsTrackTreeModel.cxx
    vsTrackTreeSelectionModel.cxx
    vsTrackTreeView.cxx
    vsTrackTreeWidget.cxx
    vsUiExtensionInterface.cxx
# TODO: Everything below should be moved to either plugins or the application
    # Query formulation (viqui integration)
    vsQfDialog.cxx
    vsQfVideoPlayer.cxx
    vsQfVideoSource.cxx
    vsVideoPlayerModel.cxx
    # Built-in live descriptors - TODO: move to application?
    vsTripwireDescriptor.cxx
)

set(vspUserInterface_AMUI
    eventTree.ui
    noteTree.ui
    trackTree.ui
    vsp.ui
)

set(vspUserInterface_UI
    alertEditor.ui
    eventInfo.ui
    trackColor.ui
    ${vspUserInterface_AMUI}
)

set(vspUserInterface_MocHeaders
    # Core code
    vsApplication.h
    vsCore.h
    vsMainWindow.h
    vsMainWindowPrivate.h
    vsScene.h
    # Supporting code
    vsAlertEditor.h
    vsAlertList.h
    vsContourWidget.h
    vsEventDataModel.h
    vsEventInfoWidget.h
    vsEventTreeModel.h
    vsEventTreeSelectionModel.h
    vsEventTreeView.h
    vsEventTreeWidget.h
    vsNoteTreeWidget.h
    vsRegionList.h
    vsTrackColorDialog.h
    vsTrackTreeModel.h
    vsTrackTreeSelectionModel.h
    vsTrackTreeView.h
    vsTrackTreeWidget.h
# TODO: Everything below should be moved to either plugins or the application
    # Query formulation (viqui integration)
    vsQfDialog.h
    vsQfVideoPlayer.h
    vsQfVideoSource.h
    # Built-in live descriptors - TODO: move to application?
    vsTripwireDescriptor.h
)

set(vspUserInterface_SdkHeaders
    # Core code
    vsApplication.h
    vsCore.h
    vsMainWindow.h
    vsScene.h
    # Supporting code
    vsEventUserInfo.h
    vsUiExtensionInterface.h
)

# END source file lists

###############################################################################

# BEGIN build rules

qt4_wrap_ui(vspUserInterface_UiSources ${vspUserInterface_UI})
qte_amc_wrap_ui(vspUserInterface_AmSources ActionManagerDialog
  ${vspUserInterface_AMUI}
)
qt4_wrap_cpp(vspUserInterface_MocSources ${vspUserInterface_MocHeaders})

source_group("User Interface" FILES
  ${vspUserInterface_UI}
)

source_group("Generated" FILES
  ${vspUserInterface_UiSources}
  ${vspUserInterface_AmSources}
  ${vspUserInterface_MocSources}
)

set_source_files_properties(${vspUserInterface_Sources}
  PROPERTIES OBJECT_DEPENDS
  "${vspUserInterface_UiSources};${vspUserInterface_AmSources}"
)

add_library(${PROJECT_NAME}
  ${vspUserInterface_Sources}
  ${vspUserInterface_UiSources}
  ${vspUserInterface_AmSources}
  ${vspUserInterface_MocSources}
)

target_link_libraries(${PROJECT_NAME}
  LINK_PUBLIC
  ${QT_QTMAIN_LIBRARY}
  ${QT_LIBRARIES}
  LINK_PRIVATE
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkCommonSystem
  vtkImagingStencil
  vtkGUISupportQt
)

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  qtExtensions
  vtkVgCore
  vtkVgModelView
  vgVtkVideo
  vgGuiFramework
  vspData
  PRIVATE_INTERFACE_TARGETS
  vgCommon
  qtVgCommon
  qtVgWidgets
  vvIO
  vvWidgets
  vtkVgQtUtil
  ${TESTING_INTERFACES}
# TODO: Only needed for code that should be moved out
  vtkVgSceneGraph
)

install_library_targets(${PROJECT_NAME})
install_headers(${vspUserInterface_SdkHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VspUserInterface)

# END build rules
