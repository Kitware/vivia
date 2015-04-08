# Options for optional plugins
vg_option(VSPUI_ENABLE_REPORT_GENERATOR ON
  "Enable building of VSPUI report generator extension"
  "VISGUI_ENABLE_VSPLAY"
)

vg_option(
  VSPUI_ENABLE_POWERPOINT_GENERATOR ON
  "Enable building of VSPUI powerpoint generator extension"
  "VISGUI_ENABLE_VSPLAY;VISGUI_ENABLE_KWPPT"
)

vg_option(
  VSPUI_ENABLE_KML_EXPORT ON
  "Enable building of VSPUI kml export extension"
  "VISGUI_ENABLE_VSPLAY"
)

vg_option(
  VSPUI_ENABLE_EVENT_CREATION_TOOLS ON
  "Enable building of VSPUI event creation tools extension"
  "VISGUI_ENABLE_VSPLAY"
)

vg_option(VSPUI_ENABLE_CONTEXT_VIEWER ON
  "Enable building of VSPUI context viewer extension"
  "VISGUI_ENABLE_VSPLAY"
)

vg_option(VSPUI_ENABLE_TIMELINE_VIEWER ON
  "Enable building of VSPUI timeline viewer extension"
  "VISGUI_ENABLE_VSPLAY"
)

if(NOT VISGUI_ENABLE_VSPLAY)
  # If vsPlay is are disabled, we are only here to declare our options, so
  # leave now
  return()
endif()

###############################################################################

add_subdirectory(Ruler)

if(VSPUI_ENABLE_REPORT_GENERATOR OR VSPUI_ENABLE_KML_EXPORT)
  add_subdirectory(ReportGenerator)
endif()

if(VSPUI_ENABLE_POWERPOINT_GENERATOR)
  add_subdirectory(PowerPointGenerator)
endif()

if(VSPUI_ENABLE_EVENT_CREATION_TOOLS)
  add_subdirectory(EventCreationTools)
endif()

if(VSPUI_ENABLE_CONTEXT_VIEWER)
  add_subdirectory(ContextViewer)
endif()

if(VSPUI_ENABLE_TIMELINE_VIEWER)
  add_subdirectory(TimelineViewer)
endif()
