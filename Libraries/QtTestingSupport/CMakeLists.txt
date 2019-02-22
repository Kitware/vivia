project(qtTestingSupport)

link_directories(${QtTesting_LIBRARY_DIR})

qt5_wrap_cpp(qtTestingSupportMocSrcs
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

add_library(${PROJECT_NAME}
  ${qtTestingSupportSrcs}
  ${qtTestingSupportMocSrcs}
)

target_include_directories(${PROJECT_NAME} SYSTEM
  PUBLIC
  ${QtTesting_INCLUDE_DIRS}
  ${vtkGUISupportQt_INCLUDE_DIRS}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  ${QtTesting_LIBRARIES}
  PRIVATE
  qtExtensions
  qtVgWidgets
  vtkGUISupportQt
  vtkRenderingCore
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkTestingRendering
  vtksys
)

install_library_targets(${PROJECT_NAME})
