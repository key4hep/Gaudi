INCLUDE(Configuration)

SET(ROOT_native_version ${ROOT_config_version})
SET(ROOT_base ${LCG_releases}/ROOT/${ROOT_native_version})
SET(ROOT_home ${ROOT_base}/${LCG_platform}/root)

SET(ROOT_FOUND 1)
SET(ROOT_INCLUDE_DIRS ${ROOT_home}/include)
SET(ROOT_LIBRARY_DIRS ${ROOT_home}/lib)
SET(ROOT_LIBRARIES Core Cint Reflex dl)

SET(ROOT_genreflex_cmd ${ROOT_home}/bin/genreflex)
SET(ROOT_genmap_cmd ${ROOT_home}/bin/genmap)


MACRO (ROOT_GENERATE_DICTIONARY INFILES LINKDEF_FILE OUTFILE INCLUDE_DIRS_IN)  
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
ENDMACRO (ROOT_GENERATE_DICTIONARY)

SET(ROOT_environment ROOTSYS=${ROOT_home}
                     PATH+=${ROOT_home}/bin
                     LD_LIBRARY_PATH+=${ROOT_home}/lib
                     PYTHONPATH+=${ROOT_home}/lib )

