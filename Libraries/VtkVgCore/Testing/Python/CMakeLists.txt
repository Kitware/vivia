set(TESTS_WITHOUT_BASELINES
  TimeStamp
)

set(TESTS_WITH_BASELINES
  MrjImageReader
)

foreach(test ${TESTS_WITHOUT_BASELINES})
  vg_add_vtk_python_test(vtkVgCore ${test})
endforeach()

foreach(test ${TESTS_WITH_BASELINES})
  vg_add_vtk_python_test(vtkVgCore ${test} WITH_BASELINE)
endforeach()
