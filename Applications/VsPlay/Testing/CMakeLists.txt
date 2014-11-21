set(TESTS_WITHOUT_BASELINES
  AboutDialog.xml
  LoadVideoArchiveCancel.xml
)

set(TESTS_WITH_BASELINES
  Annotate.xml
  Filter.xml
  LoadVideoArchive.xml
  LoadCompressedVideoArchive.xml
  MoveSlider.xml
  Select.xml
  SelectZoomReset.xml
  SelectZoom.xml
  ShowEvents.xml
  ShowEventHeads.xml
  ShowTrackBoxes.xml
  ShowTrackIDs.xml
  ShowTrackPVOs.xml
  ShowTracks.xml
  UseVideoControls-SkipBackward.xml
  UseVideoControls-SkipForward.xml
  UseVideoControls-StepBackward.xml
  UseVideoControls-StepForward.xml
  UseVideoControls-Stop.xml
)

foreach(script ${TESTS_WITHOUT_BASELINES})
  vg_add_scripted_test(vsPlay "${script}")
endforeach()

foreach(script ${TESTS_WITH_BASELINES})
  vg_add_scripted_test(vsPlay "${script}" WITH_BASELINE)
endforeach()
