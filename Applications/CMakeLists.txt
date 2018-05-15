include(${qtExtensions_USE_FILE})

# NOTE: We descend into all directories in order to allow them to define
# application-specific options alongside the application (as opposed to having
# to define the options here, where they would be less closely located to the
# code they effect). However, no other processing shall be done unless the
# condition(s) noted after the directory are met.

add_subdirectory(VpView) # if(VISGUI_ENABLE_VPVIEW)
add_subdirectory(VsPlay) # if(VISGUI_ENABLE_VSPLAY)
add_subdirectory(Viqui)  # if(VISGUI_ENABLE_VIQUI)
add_subdirectory(Web)    # if(VISGUI_ENABLE_WEB)
