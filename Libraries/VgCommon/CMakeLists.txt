project(vgCommon)

set(vgCommonSources
  vgAttributeSet.cxx
  vgActivityType.cxx
  vgEntityType.cxx
  vgEventType.cxx
)

set(vgCommonInstallHeaders
  vgAbstractFactory.h
  vgActivityType.h
  vgAttributeSet.h
  vgCheckArg.h
  vgEntityType.h
  vgEventType.h
  vgFingerprint.h
  vgFlags.h
  vgGeodesy.h
  vgGeoTypes.h
  vgNamespace.h
  vgPointerInt.h
  vgRange.h
  vgStringUtils.h
  vgTimeStamp.h
  vgTrackType.h
  vgTypes.h
  vgUtil.h
)

include_directories(
  ${PROJECT_SOURCE_DIR}
)

add_library(${PROJECT_NAME}
  ${vgCommonSources}
)

install_headers(${vgCommonInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/VgCommon)
