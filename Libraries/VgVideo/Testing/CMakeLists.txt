set(VGTEST_LINK_LIBRARIES vgVideo qtExtensions)
vg_add_test(vgVideo-BufferRaw testVideoBuffer SOURCES TestVideoBuffer.cxx)
vg_add_test(vgVideo-BufferPng testVideoBuffer ARGS "PNG")
