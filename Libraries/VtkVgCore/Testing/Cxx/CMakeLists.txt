vg_include_library_sdk_directories(qtExtensions)

set(VGTEST_LINK_LIBRARIES qtExtensions)

include_directories(${vtkVgCore_SOURCE_DIR})

vg_add_test(vtkVgCore-Instance testVtkVgInstance
            SOURCES TestInstance.cxx LINK_LIBRARIES vtkVgCore
)
vg_add_test(vtkVgCore-SharedInstance testVtkVgSharedInstance
            SOURCES TestSharedInstance.cxx
            LINK_LIBRARIES vtkVgCore
)
vg_add_test(vtkVgCore-ReadMrj testVtkVgReadMrj
            SOURCES TestReadMrj.cxx
            LINK_LIBRARIES vtkVgCore vtkTestingRendering
)
