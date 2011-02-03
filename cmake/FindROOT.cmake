include(Configuration)

set(ROOT_native_version ${ROOT_config_version})
set(ROOT_base ${LCG_releases}/ROOT/${ROOT_native_version})
set(ROOT_home ${ROOT_base}/${LCG_platform}/root)

set(ROOT_FOUND 1)
set(ROOT_INCLUDE_DIRS ${ROOT_home}/include)
set(ROOT_LIBRARY_DIRS ${ROOT_home}/lib)

set(ROOT_COMPONENTS Core Cint Reflex RIO Hist Tree Cintex Matrix GenVector MathCore MathMore XMLIO)

foreach(component ${ROOT_COMPONENTS})
  if(WIN32)
    set(ROOT_${component}_LIBRARY lib${component} )
  else()
    set(ROOT_${component}_LIBRARY ${component} )
  endif()
endforeach()

if(WIN32)
  set(ROOT_LIBRARIES ${ROOT_Core_LIBRARY} ${ROOT_Cint_LIBRARY} ${ROOT_Reflex_LIBRARY})
else()
  set(ROOT_LIBRARIES ${ROOT_Core_LIBRARY} ${ROOT_Cint_LIBRARY} ${ROOT_Reflex_LIBRARY} dl)
endif()


set(ROOT_genreflex_cmd ${ROOT_home}/bin/genreflex)
set(ROOT_genmap_cmd ${ROOT_home}/bin/genmap)


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

if(WIN32)
  set(ROOT_environment ROOTSYS=${ROOT_home}
					   PATH+=${ROOT_home}/bin
                       PYTHONPATH+=${ROOT_home}/bin )
else()
  set(ROOT_environment ROOTSYS=${ROOT_home}
					   PATH+=${ROOT_home}/bin
                       LD_LIBRARY_PATH+=${ROOT_home}/lib
                       PYTHONPATH+=${ROOT_home}/lib )
endif()
