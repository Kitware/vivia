project(vgDataFramework)

set(vgDataFrameworkSources
  vdfAbstractQuerySessionNode.cxx
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

add_library(${PROJECT_NAME} ${vgDataFrameworkSources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vvIO
  vgCommonHeaders
  qtExtensionsHeaders
  Qt5::Core
  Qt5::Concurrent
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
