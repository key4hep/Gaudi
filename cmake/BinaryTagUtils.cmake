#.rst:
# BinaryTagUtils
# --------------
#
# Utilities to work with the :variable:`BINARY_TAG` special variable.
#
#
# .. variable:: BINARY_TAG
#
#   Variable used to identify the target platform for a build. It is also used
#   to tune build paramenters, like changing from Release to Debug builds.
#
#   See https://github.com/HEP-SF/documents/tree/master/HSF-TN/draft-2015-NAM
#
#
# .. command:: parse_binary_tag
#
#   Extract components of a variable or string following
#   BINARY_TAG conventions.
#
#   Signature::
#
#     parse_binary_tag([<variable-name> [<binary-tag-string>]])
#
#   The arguments are
#
#   ``<variable-name>``
#     Name of the binary tag variable (default: ``BINARY_TAG``)
#
#   ``<binary-tag-string>``
#     Value of the binary tag string to parse (default: ``${<variable-name>}``)
#
#   This command sets a few variables in the caller scope:
#
#   * ``<variable-name>_ARCH``
#   * ``<variable-name>_MICROARCH``
#   * ``<variable-name>_OS``
#   * ``<variable-name>_COMP``
#   * ``<variable-name>_COMP_NAME``
#   * ``<variable-name>_COMP_VERSION``
#   * ``<variable-name>_TYPE``
#   * ``<variable-name>_SUBTYPE``
#
#   If the ``<binary-tag-string>`` is specified, then ``<variable-name>`` is set too.
#
#   .. note::
#
#     If no argument is passed and :variable:`BINARY_TAG` is not set we try to guess
#     it from the environment variables ``BINARY_TAG`` and ``CMTCONFIG``, or from
#     system inspection.
#
macro(parse_binary_tag)
  # parse arguments
  if(${ARGC} GREATER 0)
    set(_variable ${ARGV0})
    if(${ARGC} GREATER 1)
      set(${_variable} "${ARGV1}")
    endif()
  else()
    set(_variable BINARY_TAG)
    # try to get a value from the environment
    if(NOT BINARY_TAG)
      set(BINARY_TAG $ENV{BINARY_TAG})
    endif()
    if(NOT BINARY_TAG)
      set(BINARY_TAG $ENV{CMTCONFIG})
    endif()
    if(NOT BINARY_TAG)
      get_host_binary_tag(BINARY_TAG)
    endif()
  endif()

  string(REGEX MATCHALL "[^-]+" _out "${${_variable}}")
  list(GET _out 0 ${_variable}_ARCH)
  list(GET _out 1 ${_variable}_OS)
  list(GET _out 2 ${_variable}_COMP)
  list(GET _out 3 ${_variable}_TYPE)

  if(${_variable}_ARCH MATCHES "\\+")
    string(REGEX MATCHALL "[^+]+" ${_variable}_MICROARCH "${${_variable}_ARCH}")
    list(GET ${_variable}_MICROARCH 0 ${_variable}_ARCH)
    list(REMOVE_AT ${_variable}_MICROARCH 0)
  else()
    set(${_variable}_MICROARCH)
  endif()

  if(${_variable}_COMP MATCHES "([^0-9.]+)([0-9.]+)")
    set(${_variable}_COMP_NAME    ${CMAKE_MATCH_1})
    set(${_variable}_COMP_VERSION ${CMAKE_MATCH_2})
    if(NOT ${_variable}_COMP_NAME STREQUAL "icc")
      # all known compilers except icc have one digit per version level
      # so we map "XY" to "X.Y"
      string(REGEX MATCHALL "[0-9]" _out "${${_variable}_COMP_VERSION}")
      set(${_variable}_COMP_VERSION)
      list(GET _out 0 ${_variable}_COMP_VERSION)
      list(REMOVE_AT _out 0)
      foreach(_n ${_out})
        set(${_variable}_COMP_VERSION "${${_variable}_COMP_VERSION}.${_n}")
      endforeach()
    endif()
  else()
    set(${_variable}_COMP_NAME    ${${_variable}_COMP})
    set(${_variable}_COMP_VERSION "")
  endif()

  if(${_variable}_TYPE MATCHES "\\+")
    string(REGEX MATCHALL "[^+]+" ${_variable}_SUBTYPE "${${_variable}_TYPE}")
    list(GET ${_variable}_SUBTYPE 0 ${_variable}_TYPE)
    list(REMOVE_AT ${_variable}_SUBTYPE 0)
  else()
    set(${_variable}_SUBTYPE)
  endif()

#  foreach(_n ${_variable}
#             ${_variable}_ARCH ${_variable}_MICROARCH
#             ${_variable}_OS
#             ${_variable}_COMP ${_variable}_COMP_NAME ${_variable}_COMP_VERSION
#             ${_variable}_TYPE)
#    message(STATUS "${_n} -> ${${_n}}")
#  endforeach()
endmacro()


#.rst
# .. command:: check_compiler
#
#   make sure the current defined compiler matches the prescription
#   of BINARY_TAG
#
function(check_compiler)
  if(NOT BINARY_TAG_COMP_NAME)
    parse_binary_tag()
  endif()

  if(BINARY_TAG_COMP_NAME STREQUAL "gcc" AND NOT CMAKE_COMPILER_IS_GNUCXX)
    message(WARNING "BINARY_TAG states compiler is gcc, but ${CMAKE_CXX_COMPILER} is not GNU")
  elseif(BINARY_TAG_COMP_NAME STREQUAL "clang" AND NOT CMAKE_CXX_COMPILER MATCHES "clang")
    message(WARNING "BINARY_TAG states compiler is clang, but ${CMAKE_CXX_COMPILER} is not")
  elseif(BINARY_TAG_COMP_NAME STREQUAL "icc" AND NOT CMAKE_CXX_COMPILER MATCHES "icpc")
    message(WARNING "BINARY_TAG states compiler is Intel, but ${CMAKE_CXX_COMPILER} is not")
  endif()

  if(BINARY_TAG_COMP_VERSION)
    if(NOT BINARY_TAG_COMP_VERSION STREQUAL CMAKE_CXX_COMPILER_VERSION)
      string(LENGTH "${BINARY_TAG_COMP_VERSION}." len)
      string(SUBSTRING "${CMAKE_CXX_COMPILER_VERSION}" 0 ${len} short_version)
      if(NOT "${BINARY_TAG_COMP_VERSION}." STREQUAL short_version)
        message(WARNING "BINARY_TAG specifies compiler version ${BINARY_TAG_COMP_VERSION}. but we got ${CMAKE_CXX_COMPILER_VERSION}")
      endif()
    endif()
  endif()
endfunction()

find_program(HOST_BINARY_TAG_COMMAND
             NAMES host-binary-tag get_host_binary_tag.py
             HINTS "${CMAKE_CURRENT_LIST_DIR}")
mark_as_advanced(HOST_BINARY_TAG_COMMAND)
#.rst
# .. command:: get_host_binary_tag
#
#   Usage::
#
#       get_host_binary_tag(<variable> [<type>])
#
#   Compute host binary tag according to the rules defined in
#   https://github.com/HEP-SF/documents/tree/master/HSF-TN/draft-2015-NAM
#   and store it in the specified variable.
#
#   The argument ``<type>`` can be used a build type different from the default (opt).
#
function(get_host_binary_tag variable)
  if(ARGC GREATER 1)
    set(type ${ARGV1})
  else()
    set(type opt)
  endif()
  if(NOT HOST_BINARY_TAG_COMMAND)
    message(FATAL_ERROR "No host-binary-tag command, cannot get host binary tag")
  endif()
  if(NOT HOST_BINARY_TAG)
    execute_process(COMMAND "${HOST_BINARY_TAG_COMMAND}"
                    OUTPUT_VARIABLE HOST_BINARY_TAG
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(HOST_BINARY_TAG ${HOST_BINARY_TAG} CACHE STRING "BINARY_TAG of the host")
    mark_as_advanced(HOST_BINARY_TAG)
  endif()
  string(REGEX REPLACE "-opt\\$" "-${type}" value "${HOST_BINARY_TAG}")
  set(${variable} ${value} PARENT_SCOPE)
endfunction()

#.rst
# .. command:: compatible_binary_tags
#
#   Usage::
#
#       compatible_binary_tags(<variable>)
#
#   Set ``<variable>`` to a prioritized list of usable binary tags.
#
function(compatible_binary_tags variable)
  if(NOT BINARY_TAG_TYPE)
    parse_binary_tag()
  endif()
  set(types ${BINARY_TAG_TYPE})
  if(BINARY_TAG_TYPE STREQUAL "do0")
    list(APPEND types dbg)
  endif()
  if(NOT BINARY_TAG_TYPE STREQUAL "opt")
    list(APPEND types opt)
  endif()

  # prepare the list of archs as 'main_arch' followed by microach flags
  # e.g: arch+ma1+ma2+ma3 -> arch+ma1+ma2+ma3 arch+ma1+ma2 arch+ma1 arch
  set(archs)
  set(subarch)
  foreach(ma ${BINARY_TAG_MICROARCH})
    list(APPEND archs "${BINARY_TAG_ARCH}${subarch}")
    set(subarch "${subarch}+${ma}")
  endforeach()
  list(APPEND archs "${BINARY_TAG_ARCH}${subarch}")
  list(REVERSE archs)

  # prepare the list of build sub-types (if needed)
  set(subtypes)
  if(BINARY_TAG_SUBTYPE)
    set(subtype)
    foreach(st ${BINARY_TAG_SUBTYPE})
      set(subtype "${subtype}+${st}")
      list(APPEND subtypes "${subtype}")
    endforeach()
    list(REVERSE subtypes)
  endif()

  set(out)
  foreach(a ${archs})
    foreach(t ${types})
      foreach(st ${subtypes})
        list(APPEND out "${a}-${BINARY_TAG_OS}-${BINARY_TAG_COMP}-${t}${st}")
      endforeach()
      # the list of subtypes might be empty, so we explicitly add the simple tag
      list(APPEND out "${a}-${BINARY_TAG_OS}-${BINARY_TAG_COMP}-${t}")
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES out)
  set(${variable} ${out} PARENT_SCOPE)
endfunction()
