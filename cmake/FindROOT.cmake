include(Configuration)

set(ROOT_native_version ${ROOT_config_version}_python${Python_config_version_twodigit})
set(ROOT_base ${LCG_releases}/ROOT/${ROOT_native_version})
set(ROOT_home ${ROOT_base}/${LCG_platform}/root)

set(ROOT_FOUND 1)
set(ROOT_INCLUDE_DIRS ${ROOT_home}/include)
set(ROOT_LIBRARY_DIRS ${ROOT_home}/lib)
if(WIN32)
  set(ROOT_LIBRARIES libCore libCint libReflex)
  set(ROOT_Reflex_LIBRARY libReflex)
  set(ROOT_RIO_LIBRARY libRIO)
  set(ROOT_Hist_LIBRARY libHist)
  set(ROOT_Tree_LIBRARY libTree)
  set(ROOT_Cintex_LIBRARY libCintex)
  set(ROOT_Matrix_LIBRARY libMatrix)
else()
  set(ROOT_LIBRARIES Core Cint Reflex dl)
  set(ROOT_Reflex_LIBRARY Reflex)
  set(ROOT_RIO_LIBRARY RIO)
  set(ROOT_Hist_LIBRARY Hist)
  set(ROOT_Tree_LIBRARY Tree)
  set(ROOT_Cintex_LIBRARY Cintex)
  set(ROOT_Matrix_LIBRARY Matrix)
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
