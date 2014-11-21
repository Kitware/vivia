# Options for optional plugins
vg_option(VVQS_ENABLE_FAKE_BACKEND ON
  "Enable building of VVQS fake back-end"
  "VISGUI_ENABLE_VVIO"
)

if(NOT VISGUI_ENABLE_VVIO)
  # If the vvIO framework is disabled, we are only here to declare our options,
  # so leave now
  return()
endif()

###############################################################################

if(VVQS_ENABLE_FAKE_BACKEND)
  add_subdirectory(FakeQueryService)
endif()
