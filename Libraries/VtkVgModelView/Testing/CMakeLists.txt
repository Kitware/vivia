# Command line tests.
include_directories(../)
set(VGTEST_LINK_LIBRARIES vtkVgModelView)

set(Tests
#     testModelViewAPI.cxx
)

set(TestsToRun ${Tests})

foreach(test ${TestsToRun})
  get_filename_component(TName ${test} NAME_WE)
  string(REPLACE "test" "" testname ${TName})
  vg_add_test(vtkVgModelView-${testname} ${TName} SOURCES ${test} ARGS ${VISGUI_DATA_ROOT}/CLIF/images.txt)
endforeach(test)
