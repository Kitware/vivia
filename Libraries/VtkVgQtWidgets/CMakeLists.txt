project(vtkVgQtWidgets)

include_directories(SYSTEM
  ${VTK_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(vtkVgQtWidgetsSources
  vtkVgRegionWidget.cxx
)

set(vtkVgQtWidgetsMocHeaders
  vtkVgRegionWidget.h
)

set(vtkVgQtWidgetsInstallHeaders
  vtkVgRegionWidget.h
)

qt5_wrap_cpp(vtkVgQtWidgetsMocSources ${vtkVgQtWidgetsMocHeaders})

add_library(${PROJECT_NAME}
  ${vtkVgQtWidgetsSources}
  ${vtkVgQtWidgetsMocSources}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vtkInteractionWidgets
  PRIVATE
  qtVgCommon
  qtExtensions
  vtkVgModelView
  vtkVgQtUtil
  ${VTK_OPENGL_RENDERING_COMPONENTS}
  vtkGUISupportQt
)

install_library_targets(${PROJECT_NAME})
install_headers(${vtkVgQtWidgetsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/vtkVgQtWidgets)
