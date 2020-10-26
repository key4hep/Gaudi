# - Try to find jemalloc
# Defines:
#
#  jemalloc_FOUND
#  jemalloc_LIBRARY
#
# Imports:
#
#  jemalloc::jemalloc
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{jemalloc_LIBRARY})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

find_library(jemalloc_LIBRARY jemalloc)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(jemalloc DEFAULT_MSG jemalloc_LIBRARY)

mark_as_advanced(jemalloc_LIBRARY)

# Modernisation: create an interface target to link against
if(TARGET jemalloc::jemalloc)
    return()
endif()
if(jemalloc_FOUND)
  add_library(jemalloc::jemalloc UNKNOWN IMPORTED)
  set_target_properties(jemalloc::jemalloc PROPERTIES IMPORTED_LOCATION "${jemalloc_LIBRARY}")
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: jemalloc::jemalloc")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  variable_watch(jemalloc_LIBRARY __deprecate_var_for_target)
endif()
