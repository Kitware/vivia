project(vtkVgModelView)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

set(vtkVgModelViewSrcs
  vtkVgActivityManager.cxx
  vtkVgCompositeEventRepresentation.cxx
  vtkVgDataSourceBase.cxx
  vtkVgEventFilter.cxx
  vtkVgEventIconRepresentation.cxx
  vtkVgEventInfo.cxx
  vtkVgEventLabelRepresentation.cxx
  vtkVgEventModel.cxx
  vtkVgEventModelCollection.cxx
  vtkVgEventRegionRepresentation.cxx
  vtkVgEventRepresentation.cxx
  vtkVgEventRepresentationBase.cxx
  vtkVgGraphModel.cxx
  vtkVgGraphRepresentation.cxx
  vtkVgMarker.cxx
  vtkVgModelBase.cxx
  vtkVgPicker.cxx
  vtkVgRepresentationBase.cxx
  vtkVgTrackFilter.cxx
  vtkVgTrackHeadRepresentation.cxx
  vtkVgTrackInfo.cxx
  vtkVgTrackLabelRepresentation.cxx
  vtkVgTrackModel.cxx
  vtkVgTrackModelCollection.cxx
  vtkVgTrackRepresentation.cxx
  vtkVgTrackRepresentationBase.cxx
  vtkVgTripWireManager.cxx
  vtkVgVideoModel.cxx
  Deprecated/vtkVgVideoFrameData.cxx
  Deprecated/vtkVgVideoModel0.cxx
  Deprecated/vtkVgVideoProviderBase.cxx
  Deprecated/vtkVgVideoRepresentation0.cxx
  Deprecated/vtkVgVideoRepresentationBase0.cxx
)

set(vtkVgModelViewInstallHeaders
  vtkVgActivityManager.h
  vtkVgCompositeEventRepresentation.h
  vtkVgDataSourceBase.h
  vtkVgEventFilter.h
  vtkVgEventIconRepresentation.h
  vtkVgEventInfo.h
  vtkVgEventLabelRepresentation.h
  vtkVgEventModelCollection.h
  vtkVgEventModel.h
  vtkVgEventRegionRepresentation.h
  vtkVgEventRepresentationBase.h
  vtkVgEventRepresentation.h
  vtkVgGraphModel.h
  vtkVgGraphRepresentation.h
  vtkVgMarker.h
  vtkVgModelBase.h
  vtkVgPickData.h
  vtkVgPicker.h
  vtkVgRepresentationBase.h
  vtkVgTimeStampBase.h
  vtkVgTrackFilter.h
  vtkVgTrackHeadRepresentation.h
  vtkVgTrackInfo.h
  vtkVgTrackLabelRepresentation.h
  vtkVgTrackModelCollection.h
  vtkVgTrackModel.h
  vtkVgTrackRepresentationBase.h
  vtkVgTrackRepresentation.h
  vtkVgTripWireManager.h
  vtkVgVideoModel.h
  Deprecated/vtkVgVideoFrameData.h
  Deprecated/vtkVgVideoMetadata.h
  Deprecated/vtkVgVideoModel0.h
  Deprecated/vtkVgVideoProviderBase.h
  Deprecated/vtkVgVideoRepresentationBase0.h
  Deprecated/vtkVgVideoRepresentation0.h
)

set_source_files_properties(
  Deprecated/vtkVgVideoFrameData
  Deprecated/vtkVgVideoMetadata
  Deprecated/vtkVgVideoModel0
  Deprecated/vtkVgVideoProviderBase
  Deprecated/vtkVgVideoRepresentation0
  Deprecated/vtkVgVideoRepresentationBase0
  PROPERTIES WRAP_EXCLUDE TRUE
)

set_source_files_properties(
  vtkVgEventInfo
  vtkVgTrackInfo
  PROPERTIES WRAP_SPECIAL TRUE
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  vtkFiltersModeling
  vtkIOCore
  vtkInteractionStyle
  vtkRenderingFreeTypeOpenGL
  vtkRenderingOpenGL
  vtkVgCore
)

# Build and link library.
vtk_module_library(${PROJECT_NAME} ${vtkVgModelViewSrcs})

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vtkVgCore
)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgModelViewInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgModelView)

vg_wrap_vtk_library(${PROJECT_NAME})
