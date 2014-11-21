set(VGTEST_LINK_LIBRARIES qtExtensions ${QT_LIBRARIES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
vg_include_library_sdk_directories(qtExtensions)

# Interactive tests
vg_add_test(testDrawers INTERACTIVE
  SOURCES TestDrawers.cxx qtEditableLabel.cxx
  MOC_HEADERS TestDrawers.h qtEditableLabel.h
  RESOURCES icons.qrc
)

vg_add_test(testExpander INTERACTIVE
  SOURCES TestExpander.cxx
  MOC_HEADERS TestExpander.h
)

vg_add_test(testDoubleSlider INTERACTIVE
  SOURCES TestDoubleSlider.cxx
  MOC_HEADERS TestDoubleSlider.h
  UI testDoubleSlider.ui
)

vg_add_test(testSqueezedLabel INTERACTIVE
  SOURCES TestSqueezedLabel.cxx
  MOC_HEADERS TestSqueezedLabel.h
)

vg_add_test(testColorButton         INTERACTIVE TestColorButton.cxx)
vg_add_test(testConfirmationDialog  INTERACTIVE TestConfirmationDialog.cxx)
vg_add_test(testGradientWidget      INTERACTIVE TestGradientWidget.cxx)
vg_add_test(testThrobber            INTERACTIVE TestThrobber.cxx)

# Automated tests
vg_add_test(qtExtensions-Kst testKst
            SOURCES TestKst.cxx ../IO/qtKstParser.cxx
            ARGS ${CMAKE_CURRENT_SOURCE_DIR}/testdata.kst
)

vg_add_test(qtExtensions-UiState testUiState TestUiState.cxx)
