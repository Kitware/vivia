project(vgDataFramework)

set(vgDataFrameworkSources
  vdfNodeBase.cxx
  vdfNodeProxy.cxx
  vdfQuerySessionNode.cxx
  vdfSelector.cxx
  vdfSelectorSet.cxx
  vdfTemporalSelector.cxx
)

set(vgDataFrameworkMocHeaders
  vdfAbstractQuerySessionNode.h
  vdfNodeBase.h
  vdfNodeProxy.h
  vdfQuerySessionNode.h
  vdfSelector.h
  vdfTemporalSelector.h
)

set(vgDataFrameworkInstallHeaders
  vdfAbstractEventNode.h
  vdfAbstractQueryResultSetNode.h
  vdfAbstractQuerySessionNode.h
  vdfAbstractTrackNode.h
  vdfNamespace.h
  vdfNodeBase.h
  vdfNodeProxy.h
  vdfQuerySessionNode.h
  vdfSelector.h
  vdfSelectorSet.h
  vdfTemporalSelector.h
)

set(vgDataFrameworkWrapObjects
  vdfNodeBase
  vdfQuerySessionNode
  vdfSelector
  vdfTemporalSelector
  BY_VALUE
  vdfSelectorType
  INTERFACES
  #vdfAbstractEventNode CANNOT WRAP, depends on vtkVgModelView
  #vdfAbstractTrackNode CANNOT WRAP, depends on vtkVgModelView
)

qt4_wrap_cpp(vgDataFrameworkMocSources ${vgDataFrameworkMocHeaders})

vg_include_library_sdk_directories(qtExtensions vgCommon)

add_library(${PROJECT_NAME}
  ${vgDataFrameworkSources}
  ${vgDataFrameworkMocSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PRIVATE_INTERFACE_TARGETS
  vvIO
  LINK_LIBRARIES
  LINK_PUBLIC
  ${QT_LIBRARIES}
)

install_library_targets(${PROJECT_NAME})
install_headers(${vgDataFrameworkInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VgDataFramework)

vg_wrap_library(${PROJECT_NAME}
  TYPESYSTEM typesystem.xml
  OBJECTS ${vgDataFrameworkWrapObjects}
  HEADERS ${vgDataFrameworkInstallHeaders}
  DEPENDS PySide:Core qtExtensions vgCommon vvIO
)

vg_add_documentation(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR} Qt)
