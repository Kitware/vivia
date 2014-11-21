# Generate export header
function(vg_generate_export_header FINAL_OUTPUT_FILE EXPORTS_FILE)
  # Generate the header
  set(OUTPUT_FILE "${PROJECT_BINARY_DIR}/.gen-export.h.tmp")
  include(${PROJECT_SOURCE_DIR}/CMake/gen-export.cmake)

  # Copy the header to its final location; this is done because the copy will
  # no-op if the file isn't changed, such that we don't rebuild unnecessarily
  configure_file("${OUTPUT_FILE}" "${FINAL_OUTPUT_FILE}" COPYONLY)
  message(STATUS "Generated export header file '${FINAL_OUTPUT_FILE}'")
endfunction()
