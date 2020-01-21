set(VGTEST_LINK_LIBRARIES qtExtensions vtkVgCoreHeaders)

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
