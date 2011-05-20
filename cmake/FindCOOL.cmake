# - Try to find COOL
# Defines:
#
#  COOL_FOUND
#  COOL_INCLUDE_DIR
#  COOL_INCLUDE_DIRS (not cached)
#  COOL_<component>_LIBRARY
#  COOL_<component>_FOUND
#  COOL_LIBRARIES (not cached)
#  COOL_PYTHON_PATH

set(_COOL_COMPONENTS CoolKernel CoolApplication)
foreach(component ${_COOL_COMPONENTS})
  find_library(COOL_${component}_LIBRARY NAMES lcg_${component})
  if (COOL_${component}_LIBRARY)
    set(COOL_${component}_FOUND 1)
    list(APPEND COOL_LIBRARIES ${COOL_${component}_LIBRARY})
  else()
    set(COOL_${component}_FOUND 0)
  endif()
endforeach()

find_path(COOL_INCLUDE_DIR CoolKernel/IDatabase.h)
find_path(COOL_PYTHON_PATH PyCool/__init__.py)

set(COOL_INCLUDE_DIRS ${COOL_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set COOL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(COOL DEFAULT_MSG COOL_INCLUDE_DIR COOL_LIBRARIES COOL_PYTHON_PATH)

mark_as_advanced(COOL_FOUND COOL_INCLUDE_DIR COOL_PYTHON_PATH)
