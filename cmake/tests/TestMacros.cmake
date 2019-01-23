# Resolve the symlinks leading up to a path even if the full path does not exist
#
# This is useful for testing CMake modules which automatically generate paths
# without actually installing the associated package or making sure that every
# path in an xxx_PATH environment variable which might exist does really exist.
#
function(resolve_path var path)
  # Common case: If the path does exist, use the standard CMake resolver
  if(EXISTS "${path}")
    get_filename_component(${var} "${path}" REALPATH)
    return()
  endif()

  # Otherwise, look for a parent directory which does exist
  message(WARNING "Asked to resolve non-existent filesystem path '${path}'")
  set(existing_root "${path}")
  while(NOT EXISTS "${existing_root}" AND NOT "${existing_root}" STREQUAL "")
    get_filename_component(existing_root "${existing_root}" DIRECTORY)
  endwhile()

  # If there is none, return the input path as-is
  if("${existing_root}" STREQUAL "")
    set(${var} "${path}")
    return()
  endif()

  # Else express the input path relatively to this existing root...
  file(RELATIVE_PATH rel_path "${existing_root}" "${path}")

  # ...then resolve the path to the existing root...
  get_filename_component(existing_root "${existing_root}" REALPATH)

  # ...and attach back the nonexistent relative path to that
  get_filename_component(${var} "${rel_path}" ABSOLUTE BASE_DIR "${existing_root}")
endfunction()


# Compare two lists of filesystem paths element-wise, using the same logic as
# assert_same_path for each element of the list.
function(assert_same_path_list var value)
  # Compare the length of the input path lists
  set(value_list ${value})
  list(LENGTH ${var} len_var)
  list(LENGTH value_list len_value)
  if(NOT ${len_var} EQUAL ${len_value})
    message(FATAL_ERROR "wrong list length for ${var}: found ${len_var} entries in '${${var}}', expected ${len_value} entries from '${value}'")
  endif()

  # Compare paths in the input lists, if any, in a pair-wise fashion
  if(${len_var} GREATER 0)
    math(EXPR len_minus_one ${len_var}-1)
    foreach(index RANGE ${len_minus_one})
      list(GET ${var} ${index} path_var)
      list(GET value_list ${index} path_value)
      resolve_path(realpath_var "${path_var}")
      resolve_path(realpath_value "${path_value}")
      if(NOT "${realpath_var}" STREQUAL "${realpath_value}")
        message(FATAL_ERROR "wrong path in ${var}: found '${realpath_var}' at index ${index} of '${${var}}', expected '${realpath_value}' from '${value}'")
      endif()
    endforeach()
  endif()
endfunction()


# Compare two filesystem paths, taking symlinks into account
function(assert_same_path var value)
  resolve_path(realpath_var "${${var}}")
  resolve_path(realpath_value "${value}")
  if(NOT "${realpath_var}" STREQUAL "${realpath_value}")
    message(FATAL_ERROR "wrong path for ${var}: found '${realpath_var}', expected '${realpath_value}'")
  endif()
endfunction()


# Check that two strings are equal
function(assert_strequal var value)
  if(NOT "${${var}}" STREQUAL "${value}")
    message(FATAL_ERROR "wrong value for ${var}: found '${${var}}', expected '${value}'")
  endif()
endfunction()


# Check that a CMake expression is true
function(assert)
  if(NOT (${ARGV}))
    message(FATAL_ERROR "(${ARGV}) is false")
  endif()
endfunction()
