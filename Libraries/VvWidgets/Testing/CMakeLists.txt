set(VGTEST_LINK_LIBRARIES vvWidgets vvIO qtExtensions)

vg_add_test(testQueryInfoLabel INTERACTIVE
  SOURCES TestQueryInfoLabel.cxx
  RESOURCES ${CMAKE_SOURCE_DIR}/Icons/stdbutton.qrc
)

vg_add_test(testScoreGradientWidget INTERACTIVE
  SOURCES TestScoreGradientWidget.cxx
  RESOURCES ${CMAKE_SOURCE_DIR}/Icons/vvwidgets.qrc
)
