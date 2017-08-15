# Options for optional plugins
option(VVQS_ENABLE_FAKE_BACKEND "Enable building of VVQS fake back-end" ON)
vg_option(VVQS_ENABLE_KWIVER_BACKEND ON
  "Enable building of VVQS fake back-end"
  "VISGUI_ENABLE_KWIVER"
)

###############################################################################

if(VVQS_ENABLE_FAKE_BACKEND)
  add_subdirectory(FakeQueryService)
endif()
if(VVQS_ENABLE_KWIVER_BACKEND)
  add_subdirectory(KipQueryService)
endif()
