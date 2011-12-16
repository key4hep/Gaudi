# - Locate the ROOT headers and libraries

find_path(ROOT_INCLUDE_DIR TROOT.h)

get_filename_component(ROOTSYS ${ROOT_INCLUDE_DIR} PATH)
set(ROOTSYS ${ROOTSYS} CACHE PATH "Location of the installation of ROOT")

set(ROOT_INCLUDE_DIRS ${ROOTSYS}/include)
set(ROOT_LIBRARY_DIRS ${ROOTSYS}/lib)

set(ROOT_COMPONENTS Core Cint Reflex RIO Hist Tree TreePlayer Cintex Matrix GenVector MathCore MathMore XMLIO)

foreach(component ${ROOT_COMPONENTS})
  find_library(ROOT_${component}_LIBRARY NAMES ${component}
               PATH ${ROOT_LIBRARY_DIRS})
  mark_as_advanced(ROOT_${component}_LIBRARY)
endforeach()

set(ROOT_LIBRARIES ${ROOT_Core_LIBRARY} ${ROOT_Cint_LIBRARY} ${ROOT_Reflex_LIBRARY})

set(ROOT_genreflex_cmd ${ROOTSYS}/bin/genreflex)
set(ROOT_genmap_cmd ${ROOTSYS}/bin/genmap)

# handle the QUIETLY and REQUIRED arguments and set COOL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ROOT DEFAULT_MSG ROOTSYS ROOT_INCLUDE_DIR)
mark_as_advanced(ROOT_FOUND ROOT_INCLUDE_DIR)

macro (ROOT_GENERATE_DICTIONARY INFILES LINKDEF_FILE OUTFILE INCLUDE_DIRS_IN)
  set(INCLUDE_DIRS)
  foreach (_current_FILE ${INCLUDE_DIRS_IN})
    set(INCLUDE_DIRS ${INCLUDE_DIRS} -I${_current_FILE})
  endforeach (_current_FILE ${INCLUDE_DIRS_IN})
  STRING(REGEX REPLACE "^(.*)\\.(.*)$" "\\1.h" bla "${OUTFILE}")
  SET (OUTFILES ${OUTFILE} ${bla})
  if (CMAKE_SYSTEM_NAME MATCHES Linux)
    ADD_CUSTOM_COMMAND(OUTPUT ${OUTFILES}
    COMMAND LD_LIBRARY_PATH=${ROOT_LIBRARY_DIR} ROOTSYS=${ROOTSYS} ${ROOT_CINT_EXECUTABLE}
    ARGS -f ${OUTFILE} -c -DHAVE_CONFIG_H ${INCLUDE_DIRS} ${INFILES} ${LINKDEF_FILE}
    DEPENDS ${INFILES} ${LINKDEF_FILE})
  else (CMAKE_SYSTEM_NAME MATCHES Linux)
    if (CMAKE_SYSTEM_NAME MATCHES Darwin)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${OUTFILES}
      COMMAND DYLD_LIBRARY_PATH=${ROOT_LIBRARY_DIR} ROOTSYS=${ROOTSYS} ${ROOT_CINT_EXECUTABLE}
      ARGS -f ${OUTFILE} -c -DHAVE_CONFIG_H ${INCLUDE_DIRS} ${INFILES} ${LINKDEF_FILE}
      DEPENDS ${INFILES} ${LINKDEF_FILE} )
    endif (CMAKE_SYSTEM_NAME MATCHES Darwin)
  endif (CMAKE_SYSTEM_NAME MATCHES Linux)
endmacro (ROOT_GENERATE_DICTIONARY)

set(ROOT_ENVIRONMENT ROOTSYS=${ROOT_home})
set(ROOT_BINARY_PATH ${ROOT_home}/bin)

if(WIN32)
  set(ROOT_PYTHON_PATH ${ROOT_home}/bin)
else()
  set(ROOT_PYTHON_PATH ${ROOT_home}/lib)
endif()
