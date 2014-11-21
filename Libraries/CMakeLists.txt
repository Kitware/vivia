# Qt dependent libraries
add_subdirectory(QtExtensions)
add_subdirectory(VgCommon)
add_subdirectory(QtVgCommon)
add_subdirectory(QtVgWidgets)

if(VISGUI_ENABLE_VSPLAY OR VISGUI_ENABLE_VIQUI OR VISGUI_ENABLE_WEB)
  set(VISGUI_ENABLE_VVIO TRUE)
  add_subdirectory(VvData)
  add_subdirectory(VvIO)
  if(VISGUI_ENABLE_VSPLAY OR VISGUI_ENABLE_VIQUI)
    if(VISGUI_ENABLE_VIDTK)
      add_subdirectory(VvVidtk)
    endif()
    add_subdirectory(VvWidgets)
  endif()
else()
  set(VISGUI_ENABLE_VVIO FALSE)
endif()

add_subdirectory(VgVideo)

# VTK dependent libraries
add_subdirectory(VtkVgCore)

if(VISGUI_ENABLE_GDAL)
  add_subdirectory(VtkVgIO)
endif(VISGUI_ENABLE_GDAL)

add_subdirectory(VtkVgModelView)
add_subdirectory(VtkVgSceneGraph)
add_subdirectory(VtkVgQtUtil)
add_subdirectory(VtkVgQtWidgets)

add_subdirectory(VtkVgVideo)
add_subdirectory(VgVtkVideo)

if(VISGUI_ENABLE_VSPLAY OR VISGUI_ENABLE_VIQUI)
  add_subdirectory(VvVtkWidgets)
endif()

if(VISGUI_ENABLE_WEB)
  add_subdirectory(VtkVwCore)
endif()

add_subdirectory(VgDataFramework)

# Qt testing support
if(QtTesting_FOUND)
  add_subdirectory(QtTestingSupport)
endif()

# vsPlay support libraries
if(VISGUI_ENABLE_VSPLAY)
  add_subdirectory(VspData)
  add_subdirectory(VspSourceUtil)
  add_subdirectory(VspUserInterface)
endif()
