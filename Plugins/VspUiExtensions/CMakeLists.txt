# Options for optional plugins
vg_option(VSPUI_ENABLE_REPORT_GENERATOR ON
  "Enable building of VSPUI report generator extension"
  "VISGUI_ENABLE_VSPLAY"
)
vg_option(VSPUI_ENABLE_EVENT_CREATION_TOOLS ON
  "Enable building of VSPUI event creation tools extension"
  "VISGUI_ENABLE_VSPLAY"
)

if(NOT VISGUI_ENABLE_VSPLAY)
  # If vsPlay is are disabled, we are only here to declare our options, so
  # leave now
  return()
endif()

###############################################################################

add_subdirectory(Ruler)

if(VSPUI_ENABLE_REPORT_GENERATOR)
  add_subdirectory(ReportGenerator)
endif()
if(VSPUI_ENABLE_EVENT_CREATION_TOOLS)
  add_subdirectory(EventCreationTools)
endif()
