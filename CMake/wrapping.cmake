set(LIST_TYPESYSTEM_SOURCES_SCRIPT
  "${CMAKE_CURRENT_LIST_DIR}/list_typesystem_sources.py"
)

###############################################################################

# Function to concatenate strings in a list into a single string
function(vg_cat VAR SEP)
  set(_result)
  foreach(_item ${ARGN})
    if(_result)
      set(_result "${_result}${SEP}${_item}")
    else()
      set(_result "${_item}")
    endif()
  endforeach()
  set(${VAR} "${_result}" PARENT_SCOPE)
endfunction()

# Function to write content to a file, without spurious changes to time stamp
function(vg_write_file PATH CONTENT)
  set(CMAKE_CONFIGURABLE_FILE_CONTENT "${CONTENT}")
  configure_file(
    "${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
    "${PATH}" @ONLY
  )
endfunction()

# Function to create a Shiboken wrapped library
function(vg_add_sbk_library NAME)
  add_library(${NAME} MODULE ${ARGN})
  set_property(TARGET ${NAME} PROPERTY PREFIX "")
  set(_flags "-DSHIBOKEN")
  if(CMAKE_COMPILER_IS_GNUCXX)
    vg_cat(_flags " "
      -Wno-cast-qual
      -Wno-missing-field-initializers
      -Wno-unused-function
      -Wno-unused-parameter
      -Wno-overloaded-virtual
      ${_flags}
    )
  endif()
  set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "${_flags}")
endfunction()

# Function to wrap a library using shiboken
function(vg_wrap_library NAME)
  if(NOT VISGUI_ENABLE_PYTHON)
    return()
  endif()

  set(_pyname ${NAME}Python)

  cmake_parse_arguments(""
    ""
    "TYPESYSTEM"
    "OBJECTS;HEADERS;DEPENDS;EXTRA_INCLUDES"
    ${ARGN}
  )

  # Get list of typesystem dependencies and build paths for the same
  set(_typesystem_depends)
  set(_typesystem_paths)
  set(_extra_typesystems)
  set(_extra_include_dirs)
  set(_extra_link_libraries)
  foreach(_dep ${_DEPENDS})
    if(NOT ${_dep}_TYPESYSTEM)
      message(SEND_ERROR "${NAME} dependency ${_dep} is not a wrapped library")
    else()
      if(IS_ABSOLUTE "${${_dep}_TYPESYSTEM}")
        list(APPEND _typesystem_depends "${${_dep}_TYPESYSTEM}")
      endif()
      get_filename_component(_dep_typesystem_name "${${_dep}_TYPESYSTEM}" NAME)
      get_filename_component(_dep_typesystem_path "${${_dep}_TYPESYSTEM}" PATH)
      list(APPEND _extra_typesystems
        "  <load-typesystem name=\"${_dep_typesystem_name}\" generate=\"no\"/>"
      )
      if(_dep_typesystem_path)
        list(APPEND _typesystem_paths "${_dep_typesystem_path}")
      endif()
      if(${_dep}_TYPESYSTEM_PATHS)
        list(APPEND _typesystem_paths "${${_dep}_TYPESYSTEM_PATHS}")
      endif()
      if(${_dep}_GLOBAL_HEADER)
        list(APPEND _EXTRA_INCLUDES "${${_dep}_GLOBAL_HEADER}")
      endif()
      list(APPEND _extra_include_dirs ${${_dep}_INCLUDE_DIRS})
      list(APPEND _extra_include_dirs ${${_dep}_WRAP_INCLUDE_DIRS})
      list(APPEND _extra_link_libraries ${${_dep}_WRAP_LINK_LIBRARIES})
    endif()
  endforeach()

  # Generate monolithic include file, as required by shiboken
  set(_global_header "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_all_sdk_headers.h")
  set(_depends)
  set(_content)
  foreach(_extra_include ${_EXTRA_INCLUDES})
    list(APPEND _content "#include <${_extra_include}>")
  endforeach()
  foreach(_hdr ${_HEADERS})
    get_filename_component(_hdr "${_hdr}" REALPATH)
    list(APPEND _depends "${_hdr}")
    list(APPEND _content "#include \"${_hdr}\"")
  endforeach()
  vg_cat(_content "\n" ${_content})
  vg_write_file("${_global_header}" "${_content}\n")

  # Get list of objects to wrap
  set(_objects)
  set(_type "object-type")
  foreach(_object ${_OBJECTS})
    if(_object STREQUAL "BY_REF")
      set(_type "object-type")
    elseif(_object STREQUAL "BY_VALUE")
      set(_type "value-type")
    elseif(_object STREQUAL "INTERFACES")
      set(_type "interface-type")
    else()
      list(APPEND _objects "  <${_type} name=\"${_object}\"/>")
    endif()
  endforeach()

  # Generate typesystem
  set(_typesystem "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_typesystem.xml")
  if(_TYPESYSTEM)
    vg_cat(EXTRA_TYPESYSTEMS "\n" "${_extra_typesystems}")
    vg_cat(EXTRA_OBJECTS "\n" "${_objects}")
    set(TYPESYSTEM_NAME "${_pyname}")

    configure_file("${_TYPESYSTEM}" "${_typesystem}")
  else()
    vg_cat(_content "\n"
      "<?xml version=\"1.0\"?>"
      "<typesystem package=\"${_pyname}\">"
      ${_extra_typesystems}
      ${_objects}
      "</typesystem>"
    )
    vg_write_file("${_typesystem}" "${_content}\n")
  endif()

  # Determine list of generated source files
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}"
            "${LIST_TYPESYSTEM_SOURCES_SCRIPT}"
            "${_typesystem}"
            "${CMAKE_CURRENT_BINARY_DIR}"
    OUTPUT_VARIABLE _sources
  )
  string(REPLACE "\n" ";" _sources "${_sources}")
  set_source_files_properties(${_sources} PROPERTIES GENERATED TRUE)

  # Define rule to run the generator
  get_directory_property(_includes INCLUDE_DIRECTORIES)
  if(WIN32)
    vg_cat(_includes ";" ${_includes} ${_extra_include_dirs})
    vg_cat(_typesystem_paths ";" ${_typesystem_paths})
  else()
    vg_cat(_includes ":" ${_includes} ${_extra_include_dirs})
    vg_cat(_typesystem_paths ":" ${_typesystem_paths})
  endif()
  set(_shiboken_options
    --generatorSet=shiboken
    --enable-parent-ctor-heuristic
    --enable-return-value-heuristic
  )
  if(_DEPENDS MATCHES "PySide")
    list(APPEND _shiboken_options --enable-pyside-extensions)
  endif()
  add_custom_command(
    OUTPUT ${_sources}
    DEPENDS ${_typesystem} ${_global_header} ${_depends} ${_typesystem_depends}
    COMMAND "${SHIBOKEN_BINARY}"
            ${_shiboken_options}
            "${_global_header}"
            "--include-paths=${_includes}"
            "--typesystem-paths=${_typesystem_paths}"
            "--output-directory=${CMAKE_CURRENT_BINARY_DIR}"
            "${_typesystem}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Python bindings for ${NAME}"
  )

  # Remove "special" dependencies
  if(_DEPENDS)
    list(REMOVE_ITEM _DEPENDS PySide:Core)
    list(REMOVE_ITEM _DEPENDS PySide:Gui)
  endif()

  # Declare the wrapper library
  include_directories(SYSTEM
    ${PYTHON_INCLUDE_DIRS}
    ${SHIBOKEN_INCLUDE_DIR}
  )
  include_directories(${_extra_include_dirs})
  vg_add_sbk_library(${_pyname} ${_sources})
  target_link_libraries(${_pyname}
    PRIVATE
    ${NAME}
    ${_DEPENDS}
    ${SHIBOKEN_PYTHON_LIBRARIES}
    ${SHIBOKEN_LIBRARY}
    ${_extra_link_libraries}
  )
  foreach(_dep ${_DEPENDS})
    add_dependencies(${_pyname} ${_dep}Python)
  endforeach()

  # Install the wrapper library
  install_library_targets(${_pyname})

  # Record dependency information
  set(${NAME}_TYPESYSTEM
    "${_typesystem}"
    CACHE INTERNAL "Shiboken typesystem XML for ${NAME}"
  )
  set(${NAME}_GLOBAL_HEADER
    "${_global_header}"
    CACHE INTERNAL "Header file containing all includes for ${NAME}"
  )
  set(${NAME}_TYPESYSTEM_PATHS
    "${_typesystem_paths}"
    CACHE INTERNAL "Shiboken typesystem paths for ${NAME}"
  )
  set(${NAME}_WRAP_INCLUDE_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}/${_pyname}"
    ${_extra_include_dirs}
    CACHE INTERNAL "Include directory for wrapped ${NAME} module"
  )
endfunction()

# Function to wrap a library using VTK
function(vg_wrap_vtk_library NAME)
  if(VISGUI_ENABLE_PYTHON)
    vtk_module_load("${NAME}")
    vtk_module_headers_load("${NAME}")

    # Ensure wrapping knows about local includes, including any generated
    # headers
    #
    # NOTE: It is important that the current module's source directory is first
    # in the include order, so that wrapping finds the real headers and not the
    # <build>/include wrappers, as it only considers for wrapping objects that
    # are defined in directly-parsed headers
    set(${NAME}_INCLUDE_DIRS
      ${${NAME}_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
      ${${NAME}_INCLUDE_DIRS} # Include dirs set by vtk_module_load
      ${_old_include_dirs} # Include dirs set by VisGUI
    )
    list(REMOVE_DUPLICATES ${NAME}_INCLUDE_DIRS)

    vtk_add_python_wrapping("${NAME}")
  endif()
endfunction()
