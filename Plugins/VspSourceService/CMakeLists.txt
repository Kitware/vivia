# Options for optional plugins
vg_option(VSPSS_ENABLE_FAKE_STREAM_SOURCE ON
  "Enable building of VSPSS fake stream source"
  "VISGUI_ENABLE_VSPLAY"
)
vg_option(VSPSS_ENABLE_RANDOM_ALERT_DESCRIPTOR OFF
  "Enable building of VSPSS random alert source"
  "VISGUI_ENABLE_VSPLAY"
)

if(NOT VISGUI_ENABLE_VSPLAY)
  # If vsPlay is are disabled, we are only here to declare our options, so
  # leave now
  return()
endif()

###############################################################################

# Video source
add_subdirectory(KwaArchiveSource)

# Track source
if(TARGET track_oracle)
  add_subdirectory(TrackOracleArchiveSource)
elseif(vidtk_FOUND)
  add_subdirectory(Kw18ArchiveSource)
endif()

# Descriptor sources
add_subdirectory(PvoArchiveSource)

# Mixed-type sources
add_subdirectory(KstArchiveSource)
if(TARGET track_xgtf)
  add_subdirectory(ViperArchiveSource)
endif()

# Stream sources
if(VSPSS_ENABLE_FAKE_STREAM_SOURCE)
  add_subdirectory(FakeStreamSourceFactory)
endif()

# Live descriptors
if(VSPSS_ENABLE_RANDOM_ALERT_DESCRIPTOR)
  add_subdirectory(RandomAlertDescriptor)
endif()
