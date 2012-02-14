# - Locate RELAX libraries directory.
# Defines:
#
#  RELAX_FOUND
#  RELAX_<component>_LIBRARY
#  RELAX_LIBRARY_DIRS (not cached)

set(_RELAX_COMPONENTS
    CLHEP HepMC HepPDT MathAdd Math Reflex STLAdd STL)
foreach(component ${_RELAX_COMPONENTS})
  find_library(RELAX_${component}_LIBRARY NAMES ${component}Rflx)
  mark_as_advanced(RELAX_${component}_LIBRARY)
  if(RELAX_${component}_LIBRARY)
    list(APPEND RELAX_FOUND_COMPONENTS ${component})
    get_filename_component(libdir ${RELAX_${component}_LIBRARY} PATH)
    list(APPEND RELAX_LIBRARY_DIRS ${libdir})
  endif()
endforeach()
list(REMOVE_DUPLICATES RELAX_LIBRARY_DIRS)

# handle the QUIETLY and REQUIRED arguments and set RELAX_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RELAX DEFAULT_MSG RELAX_LIBRARY_DIRS)

mark_as_advanced(RELAX_FOUND)
