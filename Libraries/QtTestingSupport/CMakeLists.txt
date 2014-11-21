# Name of the project.
project(qtTestingSupport)

# Requires Qt and QtTesting.
include_directories(
  ${QtTesting_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${vtkGUISupportQt_INCLUDE_DIRS}
  )

link_directories( ${QtTesting_LIBRARY_DIR} )

# Source files.
qt4_wrap_cpp(qtTestingSupportMocSrcs
  pqCoreTestUtility.h
  pqFileDialogEventPlayer.h
  pqFileDialogEventTranslator.h
  pqInvokeMethodEventPlayer.h
  pqQDialogButtonBoxEventPlayer.h
  pqQDialogButtonBoxEventTranslator.h
  pqQteDoubleSliderEventPlayer.h
  pqQteDoubleSliderEventTranslator.h
  pqQVTKWidgetEventTranslator.h
  pqVgMixerEventPlayer.h
  pqVgMixerEventTranslator.h
  pqXMLEventObserver.h
  pqXMLEventSource.h
)

set(qtTestingSupportSrcs
  pqCoreTestUtility.cxx
  pqFileDialogEventPlayer.cxx
  pqFileDialogEventTranslator.cxx
  pqInvokeMethodEventPlayer.cxx
  pqQDialogButtonBoxEventPlayer.cxx
  pqQDialogButtonBoxEventTranslator.cxx
  pqQteDoubleSliderEventPlayer.cxx
  pqQteDoubleSliderEventTranslator.cxx
  pqQVTKWidgetEventPlayer.cxx
  pqQVTKWidgetEventTranslator.cxx
  pqVgMixerEventPlayer.cxx
  pqVgMixerEventTranslator.cxx
  pqXMLEventObserver.cxx
  pqXMLEventSource.cxx
)

source_group("Generated" FILES
  ${qtTestingSupportMocSrcs}
)

# Build and link library.
add_library(${PROJECT_NAME}
  ${qtTestingSupportSrcs}
  ${qtTestingSupportMocSrcs}
)

vg_add_dependencies(${PROJECT_NAME}
  LINK_LIBRARIES
  LINK_PUBLIC
  ${QT_LIBRARIES}
  ${QtTesting_LIBRARIES}
  LINK_PRIVATE
  vtkGUISupportQt
  vtkRenderingCore
  vtkRenderingFreeTypeOpenGL
  vtkTestingRendering
  vtksys
  PRIVATE_INTERFACE_TARGETS
  qtExtensions
  qtVgWidgets
)
vg_target_include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

install_library_targets(${PROJECT_NAME})
