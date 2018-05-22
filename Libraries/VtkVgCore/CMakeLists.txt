project(vtkVgCore)

if(NOT VTK_USE_SYSTEM_JPEG)
  add_definitions(-DVTK_JPEG_INCLUDE_JERROR)
endif(NOT VTK_USE_SYSTEM_JPEG)

# Requires VTK and VXL (using VIL of VXL).
include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
  ${VXL_JPEG_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
)

# Source files.
set(vtkVgCoreSrcs
  jmem_src.c
  vtkVg2DGraphMapper.cxx
  vtkVgActivity.cxx
  vtkVgActivityTypeRegistry.cxx
  vtkVgAdapt.cxx
  vtkVgAnnotationActor.cxx
  vtkVgAppendImageData.cxx
  vtkVgApplySelectedColor.cxx
  vtkVgArcParallelEdgeStrategy.cxx
  vtkVgAssignCoordinates.cxx
  vtkVgAssignCoordinatesLayoutStrategy.cxx
  vtkVgAtomic.cxx
  vtkVgBaseImageSource.cxx
  vtkVgBorderRepresentation.cxx
  vtkVgCellPicker.cxx
  vtkVgChartTimeline.cxx
  vtkVgClipPolyData.cxx
  vtkVgColorUtil.cxx
  vtkVgContourOperatorManager.cxx
  vtkVgContourRepresentation.cxx
  vtkVgContourWidget.cxx
  vtkVgCoordinateTransformUtil.cxx
  vtkVgEvent.cxx
  vtkVgEventBase.cxx
  vtkVgEventTypeRegistry.cxx
  vtkVgGeoCoord.cxx
  vtkVgGraphMapper.cxx
  vtkVgHull2D.cxx
  vtkVgIcon.cxx
  vtkVgIconManager.cxx
  vtkVgImageIcon.cxx
  vtkVgImageSource.cxx
  vtkVgInteractorStyleRubberBand2D.cxx
  vtkVgJPEGReader.cxx
  vtkVgJPEGMemoryReader.cxx
  vtkVgLabeledRegion.cxx
  vtkVgLineRepresentation.cxx
  vtkVgMetaObject.cxx
  vtkVgMultiResJpgImageReader2.cxx
  vtkVgMultiResJpgImageWriter2.cxx
  vtkVgPNGReader.cxx
  vtkVgPlotTimeline.cxx
  vtkVgQuad.cxx
  vtkVgRendererUtils.cxx
  vtkVgScalars.cxx
  vtkVgSelectionListRepresentation.cxx
  vtkVgSelectionRepresentation.cxx
  vtkVgSelectionWidget.cxx
  vtkVgSGIReader.cxx
  vtkVgSpaceConversion.cxx
  vtkVgTemporalFilters.cxx
  vtkVgTimeStamp.cxx
  vtkVgTrack.cxx
  vtkVgTrackTypeRegistry.cxx
  vtkVgTriangulateConcavePolysFilter.cxx
  vtkVgUtil.cxx
  vtkVgVideoFrame.cxx
  vtkVgVideoFrameCorner.cxx
  vtkVgVideoFrameCorners.cxx
  vtkVgVideoFrameMetaData.cxx
  vtkVgVideoSelectionSource.cxx
  vtkVgVideoSourceBase.cxx
)

set(vtkVgCoreInstallHeaders
  jmem_src.h
  vtkVg2DGraphMapper.h
  vtkVgActivity.h
  vtkVgActivityTypeRegistry.h
  vtkVgAdapt.h
  vtkVgAnnotationActor.h
  vtkVgAppendImageData.h
  vtkVgApplySelectedColor.h
  vtkVgArcParallelEdgeStrategy.h
  vtkVgAssignCoordinates.h
  vtkVgAssignCoordinatesLayoutStrategy.h
  vtkVgAtomic.h
  vtkVgBaseImageSource.h
  vtkVgBorderRepresentation.h
  vtkVgCellPicker.h
  vtkVgChartTimeline.h
  vtkVgClipPolyData.h
  vtkVgColorUtil.h
  vtkVgContourOperatorManager.h
  vtkVgContourRepresentation.h
  vtkVgContourWidget.h
  vtkVgCoordinateTransformUtil.h
  vtkVgEventBase.h
  vtkVgEvent.h
  vtkVgEventRenderSetup.h
  vtkVgEventTypeRegistry.h
  vtkVgGeoCoord.h
  vtkVgGraphMapper.h
  vtkVgHull2D.h
  vtkVgIcon.h
  vtkVgIconManager.h
  vtkVgImageIcon.h
  vtkVgImageSource.h
  vtkVgInstance.h
  vtkVgInteractorStyleRubberBand2D.h
  vtkVgJPEGReader.h
  vtkVgJPEGMemoryReader.h
  vtkVgLabeledRegion.h
  vtkVgLineRepresentation.h
  vtkVgMacros.h
  vtkVgMetaObject.h
  vtkVgMultiResJpgImageReader2.h
  vtkVgMultiResJpgImageWriter2.h
  vtkVgPlotTimeline.h
  vtkVgPNGReader.h
  vtkVgPythonUtil.h
  vtkVgQuad.h
  vtkVgRendererUtils.h
  vtkVgScalars.h
  vtkVgSelectionListRepresentation.h
  vtkVgSelectionRepresentation.h
  vtkVgSelectionWidget.h
  vtkVgSetGet.h
  vtkVgSharedInstance.h
  vtkVgSGIReader.h
  vtkVgSpaceConversion.h
  vtkVgTemporalFilters.h
  vtkVgTimeStamp.h
  vtkVgTrack.h
  vtkVgTrackTypeRegistry.h
  vtkVgTriangulateConcavePolysFilter.h
  vtkVgTypeDefs.h
  vtkVgTypeRegistryBase.h
  vtkVgTypeRegistry.h
  vtkVgUtil.h
  vtkVgVideoCache.h
  vtkVgVideoFrame.h
  vtkVgVideoFrameCorner.h
  vtkVgVideoFrameCorners.h
  vtkVgVideoFrameMetaData.h
  vtkVgVideoSelectionSource.h
  vtkVgVideoSourceBase.h
)

if(CMAKE_COMPILER_IS_GNUCXX)
  # VTK proper has a better check, but for our purposes this will suffice
  set_source_files_properties(vtkVgAtomic.cxx PROPERTIES
                              COMPILE_DEFINITIONS VTK_HAVE_SYNC_BUILTINS)
endif()

set_source_files_properties(
  vtkVgBaseImageSource
  PROPERTIES ABSTRACT TRUE
)

set_source_files_properties(
  vtkVgActivity
  vtkVgActivityTypeRegistry
  vtkVgAdapt
  vtkVgAtomic
  vtkVgColorUtil
  vtkVgCoordinateTransformUtil
  vtkVgEvent
  vtkVgEventBase
  vtkVgEventTypeRegistry
  vtkVgMetaObject
  vtkVgQuad
  vtkVgRendererUtils
  vtkVgTrackTypeRegistry
  PROPERTIES WRAP_EXCLUDE TRUE
)

set_source_files_properties(
  vtkVgGeoCoord
  vtkVgRendererUtils
  vtkVgTimeStamp
  vtkVgVideoFrame
  vtkVgVideoFrameCorner
  vtkVgVideoFrameCorners
  vtkVgVideoFrameMetaData
  PROPERTIES WRAP_SPECIAL TRUE
)

vg_vtk_module(${PROJECT_NAME}
  DEPENDS
  vtkChartsCore
  vtkFiltersHybrid
  vtkInteractionStyle
  vtkInfovisLayout
  vtkIOCore
  vtkIOImage
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkRenderingLabel
  vtkViewsCore
  vtksys
)

# Build and link library.
vtk_module_library(${PROJECT_NAME} ${vtkVgCoreSrcs})

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vgCommon
  LINK_LIBRARIES
  LINK_PUBLIC
  LINK_PRIVATE
  vgl
  vgl_algo
  vnl
  vpl
  vil
  ${VTK_JPEG_LIBRARIES}
  ${Boost_LIBRARIES}
)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgCoreInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VtkVgCore)

vg_wrap_vtk_library(${PROJECT_NAME})
