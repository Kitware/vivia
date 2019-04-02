project(vgDataFramework)

set(vgDataFrameworkSources
  vdfDataReader.cxx
  vdfDataSource.cxx
  vdfNodeBase.cxx
  vdfNodeProxy.cxx
  vdfQuerySessionNode.cxx
  vdfSelector.cxx
  vdfSelectorSet.cxx
  vdfSourceService.cxx
  vdfTemporalSelector.cxx
  vdfThreadedArchiveSource.cxx
  vdfTrackId.cxx
  vdfTrackReader.cxx
  vdfTrackSource.cxx
  vdfUtil.cxx
)

set(vgDataFrameworkMocHeaders
  vdfAbstractQuerySessionNode.h
  vdfDataReader.h
  vdfDataSource.h
  vdfNodeBase.h
  vdfNodeProxy.h
  vdfQuerySessionNode.h
  vdfSelector.h
  vdfTemporalSelector.h
  vdfTrackReader.h
  vdfTrackReaderPrivate.h
  vdfTrackSource.h
)

set(vgDataFrameworkInstallHeaders
  vdfAbstractEventNode.h
  vdfAbstractQueryResultSetNode.h
  vdfAbstractQuerySessionNode.h
  vdfAbstractTrackNode.h
  vdfArchiveSourceInfo.h
  vdfArchiveSourceInterface.h
  vdfDataReader.h
  vdfDataSource.h
  vdfDataSourceInterface.h
  vdfNamespace.h
  vdfNodeBase.h
  vdfNodeProxy.h
  vdfQuerySessionNode.h
  vdfSelector.h
  vdfSelectorSet.h
  vdfSourceService.h
  vdfTemporalSelector.h
  vdfThreadedArchiveSource.h
  vdfTrackId.h
  vdfTrackReader.h
  vdfTrackSource.h
)

set(vgDataFrameworkWrapObjects
  vdfDataReader
  vdfDataSourceInterface
  vdfNodeBase
  vdfQuerySessionNode
  vdfSelector
  vdfTemporalSelector
  vdfThreadedArchiveSource
  vdfTrackReader
  vdfTrackSourceInterface
  BY_VALUE
  vdfArchiveFileType
  vdfArchivePluginInfo
  vdfSelectorType
  INTERFACES
  #vdfAbstractEventNode CANNOT WRAP, depends on vtkVgModelView
  #vdfAbstractTrackNode CANNOT WRAP, depends on vtkVgModelView
)

qt5_wrap_cpp(vgDataFrameworkMocSources ${vgDataFrameworkMocHeaders})

vg_include_library_sdk_directories(TARGET ${PROJECT_NAME} INTERFACE
  qtExtensions
  vgCommon
)

add_library(${PROJECT_NAME}
  ${vgDataFrameworkSources}
  ${vgDataFrameworkMocSources}
)

vg_add_dependencies(${PROJECT_NAME}
  PUBLIC_INTERFACE_TARGETS
  vvIO
  LINK_LIBRARIES
  LINK_PUBLIC
  Qt5::Core Qt5::Concurrent
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

vg_add_documentation(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}
  Qt
  qtExtensions
)
