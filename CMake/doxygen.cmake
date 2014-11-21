# Doxygen functions for the visGUI project
# The following functions are defined:
#   create_doxygen
# Their syntax is:
#   create_doxygen(inputdir name [tagdep...])
#     The first argument is the directory to use as the input directory for
#     doxygen. The targets `doxygen-${name}-dir', `doxygen-${name}-doxyfile',
#     `doxygen-${name}-tag', and `doxygen-${name}' will be created. All
#     `tagdep' arguments will be added as dependencies.

vg_option(INSTALL_DOCUMENTATION ON
  "Install documentation"
  "BUILD_DOCUMENTATION" OFF
)

if(BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)
  add_custom_target(doc)

  # Find qhelpgenerator
  find_program(QHELPGENERATOR_EXECUTABLE qhelpgenerator HINTS ${QT_BINARY_DIR})

  # Find Qt tag file
  find_file(QT4_TAG_FILE
    NAMES qt4.tags qt4.tag qt.tags qt.tag
    PATHS "${QT_DOC_DIR}"
    PATH_SUFFIXES html
  )

  set(QT4_EXTRA_TAG_FILE
    "${visGUI_BINARY_DIR}/doc/qt4-extra.tag" CACHE INTERNAL
    "Location of generated file containing additional Doxygen tags for Qt4"
    FORCE
  )
endif()

# Function to create documentation
#   name - partial name of target (generates targets doxygen-${name}, etc.)
#   input_dir - directory containing files to use for documentation
#   ARGN - additional <name>s to use as tag-file dependencies
function(vg_add_documentation name input_dir)
  if(BUILD_DOCUMENTATION)
    set(tag_files)
    set(tag_targets)

    foreach(tag ${ARGN})
      if("x_${tag}" MATCHES "x_[Qq][Tt]4?")
        if(QT4_TAG_FILE)
          list(APPEND tag_files "\"${QT4_TAG_FILE}=${QT_DOC_DIR}/html\"")
        else()
          message(WARNING
            "Documentation for ${name} requested Qt tag file,"
            "but Qt tag file was not found."
          )
        endif()
        list(APPEND tag_files "\"${QT4_EXTRA_TAG_FILE}=${QT_DOC_DIR}/html\"")
        list(APPEND tag_targets doxygen-qt4-extra-tag)
      else()
        list(APPEND tag_files "\"${visGUI_BINARY_DIR}/doc/${tag}.tag=../${tag}\"")
        list(APPEND tag_targets doxygen-${tag}-tag)
      endif()
    endforeach()
    string(REPLACE ";" " " tag_files "${tag_files}")

    if(TARGET ${name})
      get_target_property(DOXYGEN_INCLUDE_PATHS
        ${name} INCLUDE_DIRECTORIES
      )
    else()
      get_directory_property(DOXYGEN_INCLUDE_PATHS
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        INCLUDE_DIRECTORIES
      )
    endif()
    string(REPLACE ";" " " DOXYGEN_INCLUDE_PATHS "${DOXYGEN_INCLUDE_PATHS}")

    set(doc_dir "${CMAKE_BINARY_DIR}/doc/${name}")
    set(doxyfile_template "${visGUI_SOURCE_DIR}/CMake/Doxyfile.in")
    set(doxyfile "${doc_dir}/Doxyfile")
    set(doc_css "${visGUI_SOURCE_DIR}/CMake/visgui.css")

    add_custom_command(
      OUTPUT "${doc_dir}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${doc_dir}"
      COMMENT "Creating documentation directory for ${name}"
    )
    add_custom_command(
      OUTPUT "${doxyfile}.tag" "${doxyfile}.html"
      COMMAND "${CMAKE_COMMAND}"
              -D "DOXYGEN_TEMPLATE=${doxyfile_template}"
              -D "DOXYGEN_PROJECT_SOURCE_DIR=${input_dir}"
              -D "DOXYGEN_DOCUMENTATION_OUTPUT_PATH=${doc_dir}"
              -D "DOXYGEN_OUTPUT_FILE=${doxyfile}"
              -D "DOXYGEN_PROJECT_NAME=${name}"
              -D "DOXYGEN_TAG_FILES=${tag_files}"
              -D "DOXYGEN_EXCLUDE_PATTERNS=${DOXYGEN_EXCLUDE_PATTERNS}"
              -D "DOXYGEN_INCLUDE_PATHS=${DOXYGEN_INCLUDE_PATHS}"
              -D "DOXYGEN_EXTRA_STYLESHEET=${doc_css}"
              -P "${visGUI_SOURCE_DIR}/CMake/doxygen-script.cmake"
      DEPENDS "${doc_dir}" "${doxyfile_template}"
      WORKING_DIRECTORY "${visGUI_BINARY_DIR}/doc/${name}"
      COMMENT "Generating Doxyfile for ${name}"
    )
    add_custom_target(doxygen-${name}-tag DEPENDS "${doxyfile}.tag")
    add_custom_command(
      TARGET doxygen-${name}-tag
      COMMAND "${DOXYGEN_EXECUTABLE}" "${doxyfile}.tag"
      WORKING_DIRECTORY "${visGUI_BINARY_DIR}/doc/${name}"
      COMMENT "Creating tags for ${name}"
    )
    add_custom_target(doxygen-${name}
      DEPENDS ${tag_targets} "${doxyfile}.html" "${doc_css}"
      COMMAND "${DOXYGEN_EXECUTABLE}" "${doxyfile}.html"
      WORKING_DIRECTORY "${visGUI_BINARY_DIR}/doc/${name}"
      COMMENT "Creating documentation for ${name}"
    )
    add_dependencies(doc doxygen-${name})

    if(VISGUI_INSTALL_DOCUMENTATION)
      install(
        DIRECTORY "${visGUI_BINARY_DIR}/doc/${name}"
        DESTINATION "share/doc/visgui-${VISGUI_VERSION_STR}/${name}"
        COMPONENT documentation
      )
    endif()

    if(QHELPGENERATOR_EXECUTABLE)
      set(qch_file "${visGUI_BINARY_DIR}/doc/${name}.qch")
      add_custom_target(doxygen-${name}-qch
        COMMAND "${QHELPGENERATOR_EXECUTABLE}" index.qhp -o "${qch_file}"
        WORKING_DIRECTORY "${visGUI_BINARY_DIR}/doc/${name}"
        DEPENDS doxygen-${name}
        COMMENT "Creating Qt compressed help for ${name}"
      )
      add_dependencies(doc doxygen-${name}-qch)

      if(VISGUI_INSTALL_DOCUMENTATION)
        install("${qch_file}"
          DESTINATION "share/doc/visgui-${VISGUI_VERSION_STR}"
          COMPONENT documentation
        )
      endif()
    endif()
  endif()
endfunction()
