find_package(PythonInterp REQUIRED)
execute_process(
  COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/FindDocUtils.py

  RESULT_VARIABLE DOCUTILS_FOUND
)
if(NOT DOCUTILS_FOUND EQUAL 0)
  message(FATAL_ERROR
    "Failed to import docutils. Please ensure that docutils is installed"
    " and that ${PYTHON_EXECUTABLE} is able to find it"
    " (adjusting your PYTHONPATH if necessary)."
  )
endif()

set(configure_file_script
  "${visGUI_SOURCE_DIR}/CMake/configure-file-script.cmake"
)

set(stylesheet "${CMAKE_CURRENT_SOURCE_DIR}/default.css")

set(user_manual_dir "${CMAKE_BINARY_DIR}/doc/user")
add_custom_command(
  OUTPUT "${user_manual_dir}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${user_manual_dir}"
)

set(create_manual_script "${CMAKE_CURRENT_BINARY_DIR}/vgCreateManual.py")

add_custom_command(
  OUTPUT "${create_manual_script}"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/vgCreateManual.py.in"
  COMMAND "${CMAKE_COMMAND}"
    "-DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/vgCreateManual.py.in"
    "-DOUTPUT_FILE=${create_manual_script}"
    "-DCONFIGURE_ARGS=@ONLY"
    "-DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}"
    "-DSTYLESHEET=${stylesheet}"
    -P "${configure_file_script}"
)

set(outputs)
foreach(app vsPlay)
  set(_in "${CMAKE_CURRENT_SOURCE_DIR}/${app}.rst")
  set(_out "${user_manual_dir}/${app}.html")
  add_custom_command(
    OUTPUT "${_out}"
    DEPENDS "${create_manual_script}" "${user_manual_dir}"
            "${_in}" ${stylesheet}
    COMMAND "${PYTHON_EXECUTABLE}" "${create_manual_script}"
      "${_in}"
      "${_out}"
      "${visGUI_SOURCE_DIR}/Icons/16x16"
      "${visGUI_SOURCE_DIR}/Icons/22x22"
      "${visGUI_SOURCE_DIR}/Plugins/VspUiExtensions/EventCreationTools/Icons/16x16"
      "${visGUI_SOURCE_DIR}/Plugins/VspUiExtensions/EventCreationTools/Icons/22x22"
  )
  list(APPEND outputs "${_out}")
endforeach()

add_custom_target(manuals ALL DEPENDS ${outputs})
