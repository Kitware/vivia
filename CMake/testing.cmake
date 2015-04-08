#------------------------------------------------------------------------------
# Add the standard testing subdirectory, if testing is enabled
function(vg_add_test_subdirectory)
  if(BUILD_TESTING)
    add_subdirectory(Testing)
  endif()
endfunction()

#------------------------------------------------------------------------------
# Define a test in the VisGUI-standard manner
function(vg_add_test)
  # Extract arguments
  extract_flags("_inter=INTERACTIVE" ${ARGN})
  if(NOT ${_inter})
    shift_arg(test_name ${ARGN})
  endif()
  shift_arg(test_executable ${ARGN})
  set(_opts "_srcs=SOURCES;_moc=MOC_HEADERS;_res=RESOURCES;_ui=UI")
  set(_opts "${_opts};_libs=LINK_LIBRARIES;_args=ARGS")
  extract_args("${_opts}" ${ARGN})
  set(_srcs ${_srcs} ${ARGN}) # Use leftover args as sources

  if(NOT TARGET ${test_executable})
    # Handle Qt executables
    if(NOT "x_${_moc}" STREQUAL "x_")
      qt4_wrap_cpp(_moc_srcs ${_moc})
      set(_srcs ${_srcs} ${_moc_srcs})
    endif()
    if(NOT "x_${_ui}" STREQUAL "x_")
      qt4_wrap_ui(_ui_srcs ${_ui})
      set(_srcs ${_srcs} ${_ui_srcs})
    endif()
    if(NOT "x_${_res}" STREQUAL "x_")
      qt4_add_resources(_res_srcs ${_res})
      set(_srcs ${_srcs} ${_res_srcs})
    endif()

    # Generate test executable
    add_executable(${test_executable} ${_srcs})
    target_link_libraries(${test_executable} ${_libs} ${VGTEST_LINK_LIBRARIES})
  endif()

  # Add CTest test, if not interactive
  if(NOT ${_inter})
    add_test(NAME ${test_name}
             COMMAND $<TARGET_FILE:${test_executable}> ${_args})
  endif()
endfunction()

#------------------------------------------------------------------------------
# Set test baseline arguments and required files
function(vg_test_set_baseline REQUIRED_FILES ARGS PATH)
  set(_baseline "${VISGUI_BASELINE_ROOT}/${PATH}")
  list(APPEND ${ARGS}
    --test-baseline "${_baseline}"
    --test-temp-root "${visGUI_BINARY_DIR}/Testing/Temporary"
  )
  set(${ARGS} "${${ARGS}}" PARENT_SCOPE)

  list(APPEND ${REQUIRED_FILES} "${_baseline}")
  set(${REQUIRED_FILES} "${${REQUIRED_FILES}}" PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------
# Get required data files from test script
function(vg_test_get_data_files SCRIPT REQUIRED_FILES ARGS)
  file(STRINGS
    "${SCRIPT}" _script_commands_with_data
    REGEX "\\$VISGUI_DATA_ROOT"
  )
  string(REGEX MATCHALL
    "[\"']\\$VISGUI_DATA_ROOT[^\"]*[\"']"
    _data_files "${_script_commands_with_data}"
  )
  if(NOT "x_${_data_files}" STREQUAL "x_")
    string(REGEX REPLACE "[\"']" "" _data_files "${_data_files}")
    string(REPLACE
      "\$VISGUI_DATA_ROOT" "${VISGUI_DATA_ROOT}"
      _data_files "${_data_files}"
    )
  endif()

  if(_data_files)
    list(APPEND ${ARGS} --test-data-root "${VISGUI_DATA_ROOT}")
    set(${ARGS} "${${ARGS}}" PARENT_SCOPE)

    list(APPEND ${REQUIRED_FILES} ${_data_files})
    set(${REQUIRED_FILES} "${${REQUIRED_FILES}}" PARENT_SCOPE)
  endif()
endfunction()

#------------------------------------------------------------------------------
# Define a standard Python test
function(vg_add_vtk_python_test MODULE NAME)
  if(VISGUI_ENABLE_PYTHON AND TARGET vtkpython)
    get_target_property(VTK_PYTHON_EXE vtkpython LOCATION)
    set(_required_files)
    set(_script "${CMAKE_CURRENT_SOURCE_DIR}/Test${NAME}.py")

    # Extract arguments
    extract_flags("_baseline=WITH_BASELINE" ${ARGN})

    # Set up test arguments
    set(_args ${ARGN})
    if(_baseline)
      vg_test_set_baseline(_required_files _args "${MODULE}/${NAME}.png")
    endif()

    # Get required data file from test script
    vg_test_get_data_files("${_script}" _required_files _args)

    # Create test
    set(_name ${MODULE}Python-${NAME})
    add_test(${_name} "${VTK_PYTHON_EXE}" "${_script}" ${_args})
    if(_required_files)
      set_tests_properties(${_name} PROPERTIES
        REQUIRED_FILES "${_required_files}"
      )
    endif()
  endif()
endfunction()

#------------------------------------------------------------------------------
# Define a standard application scripted test
function(vg_add_scripted_test APPLICATION SCRIPT)
  # Extract arguments
  extract_flags("_baseline=WITH_BASELINE" ${ARGN})
  set(_opts "_args=ARGS;_required_files=REQUIRED_FILES")
  extract_args("${_opts}" ${ARGN})

  # Get script name without extension
  get_filename_component(_name "${SCRIPT}" NAME_WE)
  get_filename_component(_script "${SCRIPT}" REALPATH)

  # Add baseline args
  if(_baseline)
    vg_test_set_baseline(_required_files _args "${APPLICATION}/${_name}.png")
  endif()

  # Get required data file from test script
  vg_test_get_data_files("${_script}" _required_files _args)

  # Define test
  add_test(NAME ${APPLICATION}-${_name}
           COMMAND $<TARGET_FILE:${APPLICATION}>
                   --test-script "${_script}" --exit ${_args}
  )
  if(NOT "x_${_required_files}" STREQUAL "x_")
    set_tests_properties(${APPLICATION}-${_name}
      PROPERTIES REQUIRED_FILES "${_required_files}"
    )
  endif()
endfunction()

#------------------------------------------------------------------------------
# Set test data location variable from environment
function(vg_set_test_path_env NAME VALUE)
  set(${NAME} "${VALUE}" CACHE PATH
      "Setting ${NAME} to '${VALUE}' from environment." FORCE
  )
endfunction()

###############################################################################

# Enable testing?
if(BUILD_TESTING)
  enable_testing()
  include(CTest)

  configure_file(${visGUI_SOURCE_DIR}/CMake/CTestCustom.ctest.in
    ${visGUI_BINARY_DIR}/CMake/CTestCustom.ctest @ONLY
  )
  file(WRITE ${visGUI_BINARY_DIR}/CTestCustom.cmake
    "INCLUDE(\"${visGUI_BINARY_DIR}/CMake/CTestCustom.ctest\")\n")

  # Configure testing data paths
  if(NOT VISGUI_DATA_ROOT OR NOT IS_DIRECTORY "${VISGUI_DATA_ROOT}")
    if(IS_DIRECTORY "$ENV{VISGUI_DATA_ROOT}")
      vg_set_test_path_env(VISGUI_DATA_ROOT "$ENV{VISGUI_DATA_ROOT}")
    else(IS_DIRECTORY "$ENV{VISGUI_DATA_ROOT}")
      set(VISGUI_DATA_ROOT VISGUI_DATA_ROOT-NOTFOUND CACHE PATH
          "The repository containing data used for VisGUI testing." FORCE
      )
      message(WARNING "VISGUI_DATA_ROOT is not valid directory. "
                      "Some tests will not be run.")
    endif(IS_DIRECTORY "$ENV{VISGUI_DATA_ROOT}")
  endif(NOT VISGUI_DATA_ROOT OR NOT IS_DIRECTORY "${VISGUI_DATA_ROOT}")

  # Configure baseline data path
  if(NOT VISGUI_BASELINE_ROOT OR NOT IS_DIRECTORY "${VISGUI_BASELINE_ROOT}")
    if(IS_DIRECTORY "$ENV{VISGUI_BASELINE_ROOT}")
      vg_set_test_path_env(VISGUI_BASELINE_ROOT "$ENV{VISGUI_BASELINE_ROOT}")
    else(IS_DIRECTORY "$ENV{VISGUI_BASELINE_ROOT}")
      set(VISGUI_BASELINE_ROOT VISGUI_BASELINE_ROOT-NOTFOUND CACHE PATH
          "The repository containing data used as a testing baseline." FORCE
      )
      message(WARNING "VISGUI_BASELINE_ROOT is not valid directory. "
                      "Some tests will not be run.")
    endif(IS_DIRECTORY "$ENV{VISGUI_BASELINE_ROOT}")
  endif(NOT VISGUI_BASELINE_ROOT OR NOT IS_DIRECTORY "${VISGUI_BASELINE_ROOT}")

  # Configure CTestCustom file
  configure_file("${CMAKE_SOURCE_DIR}/CMake/CTestCustom.ctest.in"
                 "${PROJECT_BINARY_DIR}/CMake/CTestCustom.ctest"
  )
  file(WRITE "${PROJECT_BINARY_DIR}/CTestCustom.cmake"
       "\ninclude(\"${PROJECT_BINARY_DIR}/CMake/CTestCustom.ctest\")\n"
  )

  # Configure CTest include file
  configure_file("${CMAKE_CURRENT_LIST_DIR}/CTestInclude.cmake.in"
                 "${PROJECT_BINARY_DIR}/CTestInclude.cmake" @ONLY
  )
  set_property(DIRECTORY PROPERTY
    TEST_INCLUDE_FILE "${PROJECT_BINARY_DIR}/CTestInclude.cmake"
  )

  # Check if vtkpython is available
  if(VISGUI_ENABLE_PYTHON AND NOT TARGET vtkpython)
    message(WARNING "vtkpython was not imported from VTK. "
                    "VTK Python tests will be disabled.")
  endif()
endif()
