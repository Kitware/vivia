project(vvIO)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${KML_INCLUDE_DIRS}
)

set(vvIO_Sources
  # Read/Write core
  vvHeader.cxx
  vvReader.cxx
  vvWriter.cxx
  # Read/Write file formats
  vvKmlWriter.cxx
  vvKstReader.cxx
  vvKstWriter.cxx
  vvXmlReader.cxx
  vvXmlWriter.cxx
  vvXmlUtil.cxx
  # Query Server Interface
  vvQueryInstance.cxx
  vvQueryService.cxx
  vvQuerySession.cxx
  # Utility Functions
  vvChecksum.cxx
  vvCompare.cxx
  vvJson.cxx
  vvKmlLine.cxx
  vvMakeId.cxx
  vvUtil.cxx
)

set(vvIO_MocHeaders
  # Query Server Interface
  vvQueryNotifier.h
  vvQuerySession.h
)

set(vvIOInstallHeaders
  vvChecksum.h
  vvEventSetInfo.h
  vvHeader.h
  vvJson.h
  vvKmlLine.h
  vvKmlWriter.h
  vvKstReader.h
  vvKstWriter.h
  vvMakeId.h
  vvQueryFormulation.h
  vvQueryInstance.h
  vvQueryNotifier.h
  vvQueryService.h
  vvQueryServiceInterface.h
  vvQuerySession.h
  vvReader.h
  vvTrackHash.h
  vvUtil.h
  vvWriterData.h
  vvWriter.h
  vvXmlReader.h
  vvXmlWriter.h
)

set(vvIOWrapObjects
  vvQueryResult
  BY_VALUE
  vvImageBoundingBox
  vvImagePoint
  vvImagePointF
  vvQueryInstance
  vvTrack
  vvTrackId
)

qt5_wrap_cpp(vvIO_MocSources ${vvIO_MocHeaders})

add_library(${PROJECT_NAME} ${vvIO_Sources} ${vvIO_MocSources})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  vvData
  qtVgCommon
  qtExtensions
  PRIVATE
  ${Boost_LIBRARIES}
  kml
)

vg_add_test_subdirectory()

install_library_targets(${PROJECT_NAME})
install_headers(${vvIOInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VvIO)

vg_wrap_library(${PROJECT_NAME}
  TYPESYSTEM typesystem.xml
  OBJECTS ${vvIOWrapObjects}
  HEADERS ${vvIOInstallHeaders}
  DEPENDS PySide:Core vgCommon qtExtensions
)
