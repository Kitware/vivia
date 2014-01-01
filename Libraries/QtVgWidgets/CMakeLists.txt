project(qtVgWidgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(qtvgWidgetsSources
    ctkRangeSlider.cpp
    vgAboutAction.cxx
    vgAboutDialog.cxx
    vgApplication.cxx
    vgFrameScrubber.cxx
    vgFrameSpinBox.cxx
    vgMixerDrawer.cxx
    vgMixerItem.cxx
    vgMixerWidget.cxx
    vgTextEditDialog.cxx
    vgUserManualAction.cxx
    vgVideoScrubber.cxx
)

set(qtvgWidgetsMocHeaders
    ctkRangeSlider.h
    vgAboutAction.h
    vgAboutDialog.h
    vgApplication.h
    vgFrameScrubber.h
    vgFrameSpinBox.h
    vgMixerDrawer.h
    vgMixerItem.h
    vgMixerWidget.h
    vgTextEditDialog.h
    vgUserManualAction.h
    vgVideoScrubber.h
)

set(qtVgWidgetsInstallHeaders
  vgAboutAction.h
  vgAboutDialog.h
  vgApplication.h
  vgFrameScrubber.h
  vgFrameSpinBox.h
  vgMixerWidget.h
  vgTextEditDialog.h
  vgUserManualAction.h
  vgVideoScrubber.h
)

qt4_wrap_ui(qtVgWidgetsUiSources vgAboutDialog.ui vgTextEditDialog.ui)
qt4_wrap_cpp(qtvgWidgetsMocSources ${qtvgWidgetsMocHeaders})

source_group("Generated" FILES
  ${qtVgWidgetsUiSources}
  ${qtvgWidgetsMocSources}
)

add_library(${PROJECT_NAME}
  ${qtvgWidgetsSources} ${qtVgWidgetsUiSources} ${qtvgWidgetsMocSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS qtExtensions vgCommon
  LINK_LIBRARIES LINK_PUBLIC ${QT_LIBRARIES}
)

install_library_targets(${PROJECT_NAME})
install_headers(${qtVgWidgetsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/QtVgWidgets)
