set(VGTEST_LINK_LIBRARIES vgVideo qtExtensions ${QT_LIBRARIES})
vg_add_test(vgVideo-BufferRaw testVideoBuffer SOURCES TestVideoBuffer.cxx)
vg_add_test(vgVideo-BufferPng testVideoBuffer ARGS "PNG")

vg_add_test(testKwaVideoClip INTERACTIVE SOURCES TestKwaVideoClip.cxx)
