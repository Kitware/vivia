# Command line tests.
vg_add_test(vpView-FileDataSource testFileDataSource
  ARGS ${VISGUI_DATA_ROOT}/CLIF/images/*.png ${VISGUI_DATA_ROOT}/CLIF/images.txt
  SOURCES testFileDataSource.cxx ../vpFileDataSource.cxx
  LINK_LIBRARIES vtksys vtkFiltersGeneral vtkIOCore
)

# GUI tests.
set (project_input_files
  demoFile.prj.in
)

# do path variable substitution on input files to create project files with correct paths
foreach( filename ${project_input_files} )
  string( REPLACE ".in" "" output_filename ${filename} )
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${filename}
    ${CMAKE_CURRENT_BINARY_DIR}/${output_filename}
    ESCAPE_QUOTES)
endforeach( filename )

# specify what test scripts to run
set(TESTS_WITHOUT_CONFIG
  # Dialogs
  AboutDialog.xml
  FileOpenCancel.xml
)

set(TESTS_WITHOUT_BASELINES
  # Dialogs
  OptionsDialog.xml
  # Child Windows
  3DView.xml
  Timeline.xml
  # Misc Features
  CopyImageExtents.xml
)

set(TESTS_WITH_BASELINES
  # Main Features
  OpenProject.xml
  ImportProject.xml
  StepForwardAndBack.xml
  TrackSelect.xml
  MultiProject.xml
  MultiProjectImplicitAoi.xml
  # Base Display
  ShowTracks.xml
  ShowTracksEvents.xml
  ShowTracksEventsActivities.xml
  ShowEvents.xml
  ShowEventIcons.xml
  ShowActivities.xml
  TrackTrailLength.xml
  TrackAttributeColors.xml
  TrackOverrideColor.xml
  HighlightTrack.xml
  HighlightEvent.xml
  # Filtering
  FilteringActivitiesLongTermFollowing.xml
  FilteringEventsStart.xml
  FilteringEventsStationary.xml
  FilteringEventsStop.xml
  FilteringSpacial.xml
  ProjectWithFilters.xml
  SaveLoadFilters.xml
  # Full Volume
  FullVolumeNoIcons.xml
  # Annotation
  CreateTrack.xml
  EditTrack.xml
  MergeTracksReplace1.xml
  MergeTracksReplace2.xml
  MergeTracksNew.xml
  MergeTracksMergeEvents.xml
  MergeTracksExtendEvents.xml
  # Misc Features
  Bookmark.xml
  FrameOffset.xml
  Legend.xml
  Overview.xml
  # Specific Issues
  ResetView.xml
  SwappedMultiTrackEvent.xml
)

foreach(script ${TESTS_WITHOUT_CONFIG})
  vg_add_scripted_test(vpView "${script}")
endforeach()

set(TEST_CONFIG "${VISGUI_DATA_ROOT}/Support/PerSEASEvents.ini")

foreach(script ${TESTS_WITHOUT_BASELINES})
  vg_add_scripted_test(vpView "${script}"
    REQUIRED_FILES "${TEST_CONFIG}"
    ARGS --reset-config --config "${TEST_CONFIG}"
  )
endforeach()

foreach(script ${TESTS_WITH_BASELINES})
  vg_add_scripted_test(vpView "${script}" WITH_BASELINE
    REQUIRED_FILES "${TEST_CONFIG}"
    ARGS --reset-config --config "${TEST_CONFIG}"
  )
endforeach()
