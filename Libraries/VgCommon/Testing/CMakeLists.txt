vg_include_library_sdk_directories(qtExtensions)
set(VGTEST_LINK_LIBRARIES qtExtensions vgCommon ${QT_LIBRARIES})

vg_add_test(vgCommon-Geodesy testVgGeodesy SOURCES TestGeodesy.cxx)
vg_add_test(vgCommon-Timestamp testVgTimestamp SOURCES TestTimestamp.cxx)
vg_add_test(vgCommon-PointerInt testVgPointerInt SOURCES TestPointerInt.cxx)
vg_add_test(vgCommon-AttributeSet testVgAttributeSet SOURCES TestAttributeSet.cxx)
