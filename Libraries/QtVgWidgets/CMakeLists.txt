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

add_library(${PROJECT_NAME} ${qtvgWidgetsSources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  qtExtensions
  vgCommon
)

install_library_targets(${PROJECT_NAME})
install_headers(${qtVgWidgetsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/QtVgWidgets)
