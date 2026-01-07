project(Examples)

# Use the include path and library for Qt that is used by VTK.
include_directories(
  ${VTK_INCLUDE_DIRS}
  ${VIDTK_INCLUDE_DIRS}
  ${VIDTK_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${vtkVgCore_SOURCE_DIR}
)

# Set your files and resources here
set(ExampleSrcs
  RenderVolumeWithImageActor
  GenerateBeachTestData
  TestImageIcon
)

set(ExamplesHeaders
)

#SET(ExamplesResources Icons/icons.qrc)

qt5_wrap_ui(UISrcs
  ${ExamplesUI}
)
qt5_wrap_cpp(MOCSrcs
  ${ExamplesHeaders}
)
qt5_add_resources(ResourceSrcs
  ${ExamplesResources}
)

source_group("Resources" FILES
  ${ExamplesUI}
  ${EXE_ICON}
)

source_group("Generated" FILES
  ${UISrcs}
  ${MOCSrcs}
  ${ResourceSrcs}
  ${RCS_SOURCES}
)

set_source_files_properties(${vgViewSrcs}
  PROPERTIES OBJECT_DEPENDS "${UISrcs}"
)

# It's nice to have the ui in the windows project file...just double click on it
# and designer comes up on that ui file :)
IF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
  SET (Examples ${Examples} ${ExamplesUI})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
IF (${CMAKE_BUILD_TOOL} MATCHES "devenv")
  SET (ExamplesSrcs ${ExamplesSrcs} ${ExamplesUI})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "devenv")

#add_executable(Examples
#  MACOSX_BUNDLE ${ExamplesSrcs} ${UISrcs} ${MOCSrcs} ${ResourceSrcs}
#)

#target_link_libraries( Examples
#  QVTK
#  vtkRendering
#  vtkVgCore
#  vidtk_tracking
#  vidtk_utilities_no_process
#  vgl
#  vnl
#  vul
#)
foreach(example ${ExampleSrcs})
  add_executable(${example} ${example}.cxx)
  target_link_libraries(${example} 
    vtkGUISupportQt
    vtkRendering
    vtkVgCore
    vidtk_tracking
    vidtk_utilities_no_process
    vgl
    vnl
    vul
  )
endforeach()

