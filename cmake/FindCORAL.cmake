# - Try to find CORAL
# Defines:
#
#  CORAL_FOUND
#  CORAL_INCLUDE_DIR
#  CORAL_INCLUDE_DIRS (not cached)
#  CORAL_<component>_LIBRARY
#  CORAL_<component>_FOUND
#  CORAL_LIBRARIES (not cached)

set(_CORAL_COMPONENTS CoralBase CoralKernel)
foreach(component ${_CORAL_COMPONENTS})
  find_library(CORAL_${component}_LIBRARY NAMES lcg_${component})
  if (CORAL_${component}_LIBRARY)
    set(CORAL_${component}_FOUND 1)
    list(APPEND CORAL_LIBRARIES ${CORAL_${component}_LIBRARY})
  else()
    set(CORAL_${component}_FOUND 0)
  endif()
  mark_as_advanced(CORAL_${component}_LIBRARY)
endforeach()

find_path(CORAL_INCLUDE_DIR RelationalAccess/ConnectionService.h)

set(CORAL_INCLUDE_DIRS ${CORAL_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set CORAL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CORAL DEFAULT_MSG CORAL_INCLUDE_DIR CORAL_LIBRARIES)

mark_as_advanced(CORAL_FOUND CORAL_INCLUDE_DIR)
