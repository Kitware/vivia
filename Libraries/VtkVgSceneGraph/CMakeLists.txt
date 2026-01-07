project(vtkVgSceneGraph)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vtkVgSceneGraphSrcs
  vtkVgAreaPicker.cxx
  vtkVgComputeBounds.cxx
  vtkVgFindNode.cxx
  vtkVgFindNodeVisitor.cxx
  vtkVgGeode.cxx
  vtkVgGroupNode.cxx
  vtkVgLeafNodeBase.cxx
  vtkVgNodeBase.cxx
  vtkVgNodeVisitor.cxx
  vtkVgNodeVisitorBase.cxx
  vtkVgPropCollection.cxx
  vtkVgRenderer.cxx
  vtkVgSceneManager.cxx
  vtkVgTerrain.cxx
  vtkVgTrackNode.cxx
  vtkVgTransformNode.cxx
  vtkVgVideoNode.cxx
  vtkVgVideoViewer.cxx
  vtkVgViewBase.cxx
  vtkVgViewer.cxx
  vtkVgViewerBase.cxx
)

set(vtkVgSceneGraphInstallHeaders
  vtkVgAreaPicker.h
  vtkVgComputeBounds.h
  vtkVgFindNode.h
  vtkVgFindNodeVisitor.h
  vtkVgGeode.h
  vtkVgGroupNode.h
  vtkVgLeafNodeBase.h
  vtkVgNodeBase.h
  vtkVgNodeVisitorBase.h
  vtkVgNodeVisitor.h
  vtkVgPropCollection.h
  vtkVgRenderer.h
  vtkVgSceneManager.h
  vtkVgTerrain.h
  vtkVgTrackNode.h
  vtkVgTransformNode.h
  vtkVgVideoNode.h
  vtkVgVideoViewer.h
  vtkVgViewBase.h
  vtkVgViewerBase.h
  vtkVgViewer.h
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkInteractionStyle
)

# Build and link library.
vtk_module_library(${PROJECT_NAME} ${vtkVgSceneGraphSrcs})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vtkVgModelView
)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgSceneGraphInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgSceneGraph)
