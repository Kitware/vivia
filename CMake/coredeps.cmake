# VTK is required
set(VTK_REQUIRED_COMPONENTS
  # Core components
  vtkChartsCore
  vtkCommonCore
  vtkFiltersCore
  vtkImagingCore
  vtkInfovisCore
  vtkIOCore
  vtkViewsCore
  # Additional features
  vtkCommonColor
  vtkCommonDataModel
  vtkCommonExecutionModel
  vtkCommonMath
  vtkCommonSystem
  vtkFiltersExtraction
  vtkFiltersGeneral
  vtkFiltersHybrid
  vtkFiltersModeling
  vtkFiltersSources
  vtkFiltersStatistics
  vtkFiltersTexture
  vtkImagingHybrid
  vtkImagingStencil
  vtkInfovisLayout
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkIOImage
  vtkIOLegacy
  vtkIOXML
  vtkRenderingAnnotation
  vtkRenderingContext2D
  vtkRenderingLabel
  vtksys
  vtkViewsContext2D
  vtkViewsInfovis
)

find_package(VTK NO_MODULE REQUIRED COMPONENTS ${VTK_REQUIRED_COMPONENTS})
include(${VTK_USE_FILE})

# We need this definition to be set regardless
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

include(vtkExternalModuleMacros)
