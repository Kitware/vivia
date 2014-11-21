# NOTE: We descend into all directories in order to allow them to define
# service- or plugin-specific options alongside the affected components (as
# opposed to having to define the options here, where they would be less
# closely located to the code they effect). However, no other processing shall
# be done unless the condition(s) noted after the directory are met.

add_subdirectory(VspSourceService)  # if(VISGUI_ENABLE_VSPLAY)
add_subdirectory(VspUiExtensions)   # if(VISGUI_ENABLE_VSPLAY)
add_subdirectory(VvQueryService)    # if(VISGUI_ENABLE_VVIO)
