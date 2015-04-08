# Options for optional plugins
option(VVQS_ENABLE_FAKE_BACKEND "Enable building of VVQS fake back-end" ON)

###############################################################################

if(VVQS_ENABLE_FAKE_BACKEND)
  add_subdirectory(FakeQueryService)
endif()
