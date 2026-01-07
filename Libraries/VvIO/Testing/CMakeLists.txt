set(VGTEST_LINK_LIBRARIES vvIO qtExtensions)

vg_add_test(vvIO-Checksum testVvChecksum SOURCES TestChecksum.cxx)

vg_add_test(vvIO-KstReadWrite testVvKstReadWrite
            SOURCES TestKstReadWrite.cxx TestReadWrite.cxx
            ARGS ${CMAKE_CURRENT_SOURCE_DIR}/rw)

vg_add_test(vvIO-XmlReadWrite testVvXmlReadWrite
            SOURCES TestXmlReadWrite.cxx TestReadWrite.cxx
            ARGS ${CMAKE_CURRENT_SOURCE_DIR}/rw)

vg_add_test(testVvRead INTERACTIVE SOURCES TestVvRead.cxx)
