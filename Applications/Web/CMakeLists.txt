vg_option(VISGUI_ENABLE_KOVSQI ON
  "Enable kovqsi"
  "VISGUI_ENABLE_WEB"
)

if(VISGUI_ENABLE_KOVSQI)
  add_subdirectory(Kovsqi)
endif()
