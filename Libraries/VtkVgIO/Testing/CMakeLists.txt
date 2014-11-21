# FIXME: For now we are pulling jpeg library via vil in vtkVgCore.
set(VGTEST_LINK_LIBRARIES vtkVgIO vtkVgCore vtkTestingRendering)

vg_add_test(vtkVgIO-GDALReader testGDALReader
            SOURCES TestGDALReader.cxx
            LINK_LIBRARIES ${VGTEST_LINK_LIBRARIES}
            ARGS -D "${VISGUI_DATA_ROOT}"
                 -T "${visGUI_BINARY_DIR}/Testing/Temporary"
                 -V "${VISGUI_BASELINE_ROOT}/20091021202634-01000197-VIS.ntf.r0"
)
