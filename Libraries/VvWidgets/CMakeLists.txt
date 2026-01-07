project(vvWidgets)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vvWidgets_Sources
  vvAbstractQueryServerChooser.cxx
  vvCustomQueryServerChooser.cxx
  vvDescriptorInfoDialog.cxx
  vvDescriptorInfoTree.cxx
  vvDescriptorInfoWidget.cxx
  vvDescriptorStyle.cxx
  vvDescriptorStyleDelegate.cxx
  vvDescriptorStyleDelegateBase.cxx
  vvDescriptorStyleSetting.cxx
  vvQueryInfoLabel.cxx
  vvQueryServerDialog.cxx
  vvScoreGradient.cxx
  vvScoreGradientSetting.cxx
  vvScoreGradientStopTree.cxx
  vvScoreGradientWidget.cxx
  vvTrackInfoDialog.cxx
  vvTrackInfoWidget.cxx
)

set(vvWidgetsInstallHeaders
  vvAbstractQueryServerChooser.h
  vvDescriptorInfoDialog.h
  vvDescriptorInfoTree.h
  vvDescriptorInfoWidget.h
  vvDescriptorStyleDelegate.h
  vvDescriptorStyle.h
  vvDescriptorStyleSetting.h
  vvQueryInfoLabel.h
  vvQueryServerDialog.h
  vvScoreGradient.h
  vvScoreGradientSetting.h
  vvScoreGradientWidget.h
  vvTrackInfoDialog.h
  vvTrackInfoWidget.h
)

add_library(${PROJECT_NAME} ${vvWidgets_Sources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vvIO
  qtVgCommon
  qtExtensions
)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vvWidgetsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvWidgets)
