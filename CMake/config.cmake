include(CheckCXXCompilerFlag)

#------------------------------------------------------------------------------
# Add first of specified flags that is supported by the compiler
function(vg_add_cxx_flags_priority)
  string(REPLACE " " ";" initial_flags ${CMAKE_CXX_FLAGS})
  foreach(flag ${ARGN})
    list(FIND initial_flags ${flag} FLAG_INDEX)
    if(NOT FLAG_INDEX EQUAL -1)
      return()
    endif()
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" varname "${flag}")
    check_cxx_compiler_flag("${flag}" CMAKE_CXX_COMPILER_SUPPORTS_${varname})
    if(CMAKE_CXX_COMPILER_SUPPORTS_${varname})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

#------------------------------------------------------------------------------
# Add specified flags, if supported by the compiler
function(vg_add_cxx_flags)
  foreach(flag ${ARGN})
    vg_add_cxx_flags_priority("${flag}")
  endforeach()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
endfunction()

###############################################################################

# Require C++11 compiler
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Set default visibility to hidden when building shared
if(BUILD_SHARED_LIBS)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
  if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
  endif()
else()
  add_definitions(-DVISGUI_STATIC)
endif()

# Set extra compiler flags
if(NOT MSVC)
  # Determine what flags (if any) are needed for required C++ language support
  # Note: MSVC always uses latest known C++ extensions
  vg_add_cxx_flags_priority(-std=c++11 -std=c++0x)

  # Turn on extra warnings if requested
  option(VISGUI_EXTRA_WARNINGS "Enable extra warnings" ON)
  if(VISGUI_EXTRA_WARNINGS)
    vg_add_cxx_flags(
      -Wall -Wextra -Wunused -Wuninitialized -Wctor-dtor-privacy
      -Wcast-align -Wlogical-op -Wpointer-arith
    )
  endif()
  # Turn on additional warnings-as-errors
  vg_add_cxx_flags(
    # Missing 'return'
    -Werror=return-type
    # Improper use of 'virtual'
    -Werror=non-virtual-dtor -Werror=overloaded-virtual
    # Casting away 'const' without const_cast (i.e. via a C-style cast)
    -Werror=cast-qual
    # Use of ill-formed constructs
    -Werror=narrowing
    # Local and member variable initialization issues
    -Werror=init-self -Werror=reorder
  )
endif()
