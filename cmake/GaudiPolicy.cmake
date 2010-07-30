CMAKE_POLICY(SET CMP0003 NEW) # See "cmake --help-policy CMP0003" for more details
CMAKE_POLICY(SET CMP0011 NEW) # See "cmake --help-policy CMP0011" for more details

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/InstallArea )
SET(CMAKE_VERBOSE_MAKEFILES ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_COMPILER g++)

# Compilation Flags
SET(CMAKE_CXX_FLAGS "-D_GNU_SOURCE -Dunix -pipe -ansi -Wall -Wextra -pthread  -Wno-deprecated -Wwrite-strings -Wpointer-arith -Woverloaded-virtual -Wno-long-long")
IF (CMAKE_SYSTEM_NAME MATCHES Linux) 
  SET(CMAKE_CXX_FLAGS "-Dlinux ${CMAKE_CXX_FLAGS}")
ENDIF()

# Link shared flags
IF (CMAKE_SYSTEM_NAME MATCHES Linux) 
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000")
ENDIF()

IF(APPLE)
   SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -flat_namespace -single_module -undefined dynamic_lookup")
   SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -flat_namespace -single_module -undefined dynamic_lookup")
ENDIF()

IF(APPLE)
  SET(ld_library_path DYLD_LIBRARY_PATH)
ELSE()
  SET(ld_library_path LD_LIBRARY_PATH)
ENDIF() 

FIND_PROGRAM(python_cmd python)
IF(CMAKE_PROJECT_NAME STREQUAL GAUDI)
  SET(merge_rootmap_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/merge_files.py)
  SET(merge_conf_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/merge_files.py)
  SET(genconf_cmd ${CMAKE_BINARY_DIR}/GaudiKernel/genconf.exe)
ELSE()
  SET(merge_rootmap_cmd ${python_cmd} ${GAUDI_installarea}/scripts/merge_files.py)
  SET(merge_conf_cmd ${python_cmd} ${GAUDI_installarea}/scripts/merge_files.py)
  SET(genconf_cmd ${GAUDI_installarea}/bin/genconf.exe)
ENDIF()


##############################
#---REFLEX_GENERATE_DICTIONARY
##############################
MACRO(REFLEX_GENERATE_DICTIONARY dictionary _headerfiles _selectionfile)  
  FIND_PACKAGE(GCCXML)
  FIND_PACKAGE(ROOT)

  SET(options ${ARGN})
  IF( IS_ABSOLUTE ${_selectionfile}) 
   SET( selectionfile ${_selectionfile})
  ELSE() 
   SET( selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${_selectionfile}) 
  ENDIF()
  IF( IS_ABSOLUTE ${_headerfiles}) 
    SET( headerfiles ${_headerfiles})
  ELSE ()
    SET( headerfiles ${CMAKE_CURRENT_SOURCE_DIR}/${_headerfiles})
  ENDIF()
 
  SET(gensrcdict ${dictionary}_dict.cpp)
  #SET(gccxmlopts "--gccxmlopt=\'--gccxml-cxxflags -m64 \'")
  SET(gccxmlopts)
  SET(rootmapname ${dictionary}Dict.rootmap)
  SET(rootmapopts --rootmap=${rootmapname} --rootmap-lib=lib${dictionary}Dict)

  SET(include_dirs -I${CMAKE_CURRENT_SOURCE_DIR})
  GET_DIRECTORY_PROPERTY(_incdirs INCLUDE_DIRECTORIES)
  FOREACH ( d ${_incdirs})    
   SET(include_dirs ${include_dirs} -I${d})
  ENDFOREACH()
 
  IF (CMAKE_SYSTEM_NAME MATCHES Linux)    
    ADD_CUSTOM_COMMAND(
      OUTPUT ${gensrcdict} ${rootmapname}     
      COMMAND ${ROOT_genreflex_cmd}       
      ARGS ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
           --gccxmlpath=${GCCXML_home}/bin ${options} ${include_dirs}
      DEPENDS ${headerfiles} ${selectionfile})  
  ELSE () 
    ADD_CUSTOM_COMMAND(
      OUTPUT ${gensrcdict} ${rootmapname}      
      COMMAND ${ROOT_genreflex_cmd}       
      ARGS ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
           --gccxmlpath=${GCCXML_home}/bin ${include_dirs}
      DEPENDS ${headerfiles} ${selectionfile})     
  ENDIF ()
ENDMACRO()

##############################
#---REFLEX_BUILD_DICTIONARY
##############################
FUNCTION(REFLEX_BUILD_DICTIONARY dictionary headerfiles selectionfile )  
  REFLEX_GENERATE_DICTIONARY(${dictionary} ${headerfiles} ${selectionfile})
  ADD_LIBRARY(${dictionary}Dict MODULE ${gensrcdict})
  TARGET_LINK_LIBRARIES(${dictionary}Dict ${ARGN} Reflex)
  INSTALL(TARGETS ${dictionary}Dict LIBRARY DESTINATION lib)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname} DESTINATION lib)
ENDFUNCTION()

##############################
#---SET_RUNTIME_PATH
##############################
MACRO( SET_RUNTIME_PATH var)
  SET( runtime_dirs ${GAUDI_LIBRARY_DIRS})
  GET_PROPERTY(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  FOREACH( package ${found_packages} )
     FOREACH( env ${${package}_environment})
         IF(env MATCHES "LD_LIBRARY_PATH[+]=.*")
            STRING(REGEX REPLACE "LD_LIBRARY_PATH[+]=(.+)" "\\1"  val ${env})
            SET(runtime_dirs ${val} ${runtime_dirs})
         ENDIF()
     ENDFOREACH()
  ENDFOREACH()
  #MESSAGE("runtime_dirs --> ${runtime_dirs}")
  FOREACH(p ${runtime_dirs})
    IF( ${var} )
      SET(${var} ${${var}}:${p})
    ELSE()
      SET(${var} ${p})
    ENDIF()
  ENDFOREACH()
ENDMACRO()

##############################
#---GAUDI_GENERATE_ROOTMAP
##############################
FUNCTION(GAUDI_GENERATE_ROOTMAP library)
  FIND_PACKAGE(ROOT)
  SET(rootmapfile ${library}.rootmap)
  SET(fulllibname lib${library}.so)
  SET_RUNTIME_PATH(ld_path)
  ADD_CUSTOM_COMMAND( OUTPUT ${rootmapfile}
                      COMMAND ${ld_library_path}=.:${ld_path}:$ENV{${ld_library_path}} ${ROOT_genmap_cmd} -i ${fulllibname} -o ${rootmapfile} 
                      DEPENDS ${library} )
  ADD_CUSTOM_TARGET( ${library}Rootmap ALL DEPENDS  ${rootmapfile})
  #----Installation details-------------------------------------------------------
  SET(mergedRootMap ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_PROJECT_NAME}.rootmap)
  SET(srcRootMap ${CMAKE_CURRENT_BINARY_DIR}/${library}.rootmap)
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${merge_rootmap_cmd} --do-merge --input-file ${srcRootMap} --merged-file ${mergedRootMap})")
ENDFUNCTION()

################################
#---GAUDI_GENERATE_CONFIGURATION
################################
FUNCTION(GAUDI_GENERATE_CONFIGURATION library)
  GET_FILENAME_COMPONENT(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  SET(library_preload)  # TODO....
  SET(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})
  FILE(MAKE_DIRECTORY ${outdir})
  SET(confModuleName GaudiKernel.Proxy)
  SET(confDefaultName Configurable.DefaultName)
  SET(confAlgorithm ConfigurableAlgorithm)
  SET(confAlgTool ConfigurableAlgTool)
  SET(confAuditor ConfigurableAuditor)
  SET(confService ConfigurableService)
  SET_RUNTIME_PATH(ld_path)
  IF( TARGET GaudiSvc)
	  SET(GaudiSvc_dependency GaudiSvc)
  ENDIF()
  ADD_CUSTOM_COMMAND( 
    OUTPUT ${outdir}/${library}_confDb.py
    COMMAND ${ld_library_path}=.:${ld_path}:$ENV{${ld_library_path}} ${genconf_cmd} ${library_preload} -o ${outdir} -p ${package} 
						--configurable-module=${confModuleName}
            --configurable-default-name=${confDefaultName}
            --configurable-algorithm=${confAlgorithm}
 	    	 		--configurable-algtool=${confAlgTool}
 	    			--configurable-auditor=${confAuditor}
            --configurable-service=${confService}
            -i lib${library}.so 
    DEPENDS ${library} ${GaudiSvc_dependency} )
  ADD_CUSTOM_TARGET( ${library}Conf ALL DEPENDS  ${outdir}/${library}_confDb.py )
  #----Installation details-------------------------------------------------------
  SET(mergedConf ${CMAKE_INSTALL_PREFIX}/python/${CMAKE_PROJECT_NAME}_merged_confDb.py)
  SET(srcConf ${outdir}/${library}_confDb.py)
  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConf/ DESTINATION python PATTERN "*.stamp" EXCLUDE PATTERN "*.pyc" EXCLUDE )
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${merge_conf_cmd} --do-merge --input-file ${srcConf} --merged-file ${mergedConf})")
  GAUDI_INSTALL_PYTHON_INIT()  
ENDFUNCTION()

##############################
#---GAUDI_LINKER_LIBRARY
##############################
FUNCTION(GAUDI_LINKER_LIBRARY library sources )
  SET(lib_srcs)
  FOREACH( fp ${sources})
    FILE(GLOB files src/${fp})
    IF(files) 
      SET( lib_srcs ${lib_srcs} ${files})
    ELSE()
      SET( lib_srcs ${lib_srcs} ${fp})
    ENDIF()
  ENDFOREACH()
  ADD_LIBRARY( ${library} ${lib_srcs})
  SET_TARGET_PROPERTIES(${library} PROPERTIES COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY)
  TARGET_LINK_LIBRARIES(${library} ${ARGN})
  IF(TARGET ${library}Obj2doth)
    ADD_DEPENDENCIES( ${library} ${library}Obj2doth) 
  ENDIF()
  #----Installation details-------------------------------------------------------
  INSTALL(TARGETS ${library} LIBRARY DESTINATION lib)
ENDFUNCTION()

##############################
#---GAUDI_COMPONENT_LIBRARY
##############################
FUNCTION(GAUDI_COMPONENT_LIBRARY library sources )
  SET(lib_srcs)
  FOREACH( fp ${sources})  
    FILE(GLOB files src/${fp})
    SET( lib_srcs ${lib_srcs} ${files})
  ENDFOREACH()
  ADD_LIBRARY( ${library} MODULE ${lib_srcs})
  GAUDI_GENERATE_ROOTMAP(${library})
  GAUDI_GENERATE_CONFIGURATION(${library})
  TARGET_LINK_LIBRARIES(${library} Reflex ${ARGN})
  #----Installation details-------------------------------------------------------
  INSTALL(TARGETS ${library} LIBRARY DESTINATION lib)
ENDFUNCTION()

##############################
#---GAUDI_EXECUTABLE
##############################
FUNCTION(GAUDI_EXECUTABLE executable sources)
  SET(exe_srcs)
  FOREACH( fp ${sources})  
    FILE(GLOB files src/${fp})
    SET( exe_srcs ${exe_srcs} ${files})
  ENDFOREACH()
  ADD_EXECUTABLE( ${executable} ${exe_srcs})
  TARGET_LINK_LIBRARIES(${executable} ${ARGN} )
  #----Installation details-------------------------------------------------------
  INSTALL(TARGETS ${executable} RUNTIME DESTINATION bin)
ENDFUNCTION()

##############################
#---GAUDI_TEST
##############################
FUNCTION(GAUDI_TEST executable sources)
  SET(exe_srcs)
  FOREACH( fp ${sources})  
    FILE(GLOB files ${fp})
    SET( exe_srcs ${exe_srcs} ${files})
  ENDFOREACH()
  ADD_EXECUTABLE( ${executable} ${exe_srcs})
  TARGET_LINK_LIBRARIES(${executable} ${ARGN} )
  #----Installation details-------------------------------------------------------
  INSTALL(TARGETS ${executable} RUNTIME DESTINATION bin/tests)
ENDFUNCTION()

##############################
#---GAUDI_INSTALL_HEADERS
##############################
FUNCTION(GAUDI_INSTALL_HEADERS)
  IF( ARGN )
    SET( dirs ${ARGN} )
  ELSE()
    GET_FILENAME_COMPONENT(dirs ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  ENDIF()
  FOREACH( inc ${dirs})  
    INSTALL(DIRECTORY ${inc} DESTINATION include PATTERN ".svn" EXCLUDE )
  ENDFOREACH()
ENDFUNCTION()

##############################
#---GAUDI_INSTALL_PYTHON_MODULES
##############################
FUNCTION(GAUDI_INSTALL_PYTHON_MODULES)  
  INSTALL(DIRECTORY python/ DESTINATION python 
          PATTERN ".svn" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
  GAUDI_INSTALL_PYTHON_INIT()  
ENDFUNCTION()


##############################
#---GAUDI_INSTALL_PYTHON_INIT
##############################
FUNCTION(GAUDI_INSTALL_PYTHON_INIT)  
  GET_FILENAME_COMPONENT(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  INSTALL(CODE "IF (NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/python/${package}/__init__.py\")
                  FILE(INSTALL DESTINATION \"${CMAKE_INSTALL_PREFIX}/python/${package}\"
                               TYPE FILE 
                               FILES \"${GAUDI_SOURCE_DIR}/GaudiPolicy/cmt/fragments/__init__.py\"  )
                ENDIF()" )
ENDFUNCTION()


##############################
#---GAUDI_INSTALL_SCRIPTS
##############################
FUNCTION(GAUDI_INSTALL_SCRIPTS)
  INSTALL(DIRECTORY scripts/ DESTINATION scripts 
          FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ 
          PATTERN ".svn" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
ENDFUNCTION()

################################
#---GAUDI_PROJECT_VERSION_HEADER
################################
FUNCTION( GAUDI_PROJECT_VERSION_HEADER )
  SET(project ${CMAKE_PROJECT_NAME})
  SET(version ${${CMAKE_PROJECT_NAME}_VERSION})
  SET(ProjectVersionHeader_output  ${CMAKE_INSTALL_PREFIX}/include/${project}_VERSION.h)
  ADD_CUSTOM_TARGET( ${project}VersionHeader ALL
                     ${python_cmd}  ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/createProjVersHeader.py ${project} ${version} ${ProjectVersionHeader_output} )
ENDFUNCTION()


################################
#---GAUDI_USE_PACKAGE
################################
MACRO( GAUDI_USE_PACKAGE package )
  IF( EXISTS ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt)
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/${package} ) 
    FILE(READ ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt file_contents)
    STRING( REGEX MATCHALL "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])+" vars ${file_contents})
    FOREACH( var ${vars})
      STRING(REGEX REPLACE "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])" "\\1" p ${var})
      GAUDI_USE_PACKAGE(${p})
    ENDFOREACH()
  ELSE()
    FIND_PACKAGE(${package})
    GET_PROPERTY(parent DIRECTORY PROPERTY PARENT_DIRECTORY)
    IF(parent)
      SET(${package}_environment  ${${package}_environment} PARENT_SCOPE)
    ELSE()
      SET(${package}_environment  ${${package}_environment} )
    ENDIF()
    INCLUDE_DIRECTORIES( ${${package}_INCLUDE_DIRS} ) 
    LINK_DIRECTORIES( ${${package}_LIBRARY_DIRS} ) 
  ENDIF()
ENDMACRO()

################################
#---GAUDI_BUILD_SETUP
################################
FUNCTION( GAUDI_BUILD_SETUP )
  GAUDI_BUILD_PROJECT_SETUP()
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  MESSAGE("Found packages = ${found_packages}")
  FOREACH( package ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( "" ${package} "${${package}_environment}")
  ENDFOREACH()
ENDFUNCTION()

################################
#---GAUDI_BUILD_PROJECT_SETUP
################################
FUNCTION( GAUDI_BUILD_PROJECT_SETUP )
  SET( setup  ${CMAKE_INSTALL_PREFIX}/setup.csh )
  FILE(WRITE  ${setup} "# ${CMAKE_PROJECT_NAME} Setup file\n")
  FILE(APPEND ${setup} "setenv PATH  ${CMAKE_INSTALL_PREFIX}/bin:${CMAKE_INSTALL_PREFIX}/scripts:\${PATH}\n")
  FILE(APPEND ${setup} "setenv LD_LIBRARY_PATH  ${CMAKE_INSTALL_PREFIX}/lib:\${LD_LIBRARY_PATH}\n")
  FILE(APPEND ${setup} "setenv PYTHONPATH  ${CMAKE_INSTALL_PREFIX}/python:\${PYTHONPATH}\n")
  FILE(APPEND ${setup} "setenv JOBOPTSEARCHPATH  ${CMAKE_SOURCE_DIR}/GaudiPoolDb/options:${CMAKE_SOURCE_DIR}/GaudiExamples/options\n")

  #----Get the setup fro each external package
  GET_PROPERTY(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  #MESSAGE("Found packages = ${found_packages}")
  FOREACH( package ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${package} "${${package}_environment}")
  ENDFOREACH()

  #---Get the setup for each package (directory)
  FILE(APPEND  ${setup} "\n# Standard variables for each package\n")
  FILE(GLOB_RECURSE cmakelist_files  ${CMAKE_SOURCE_DIR} CMakeLists.txt)
  FOREACH( file ${cmakelist_files} )
    GET_FILENAME_COMPONENT(path ${file} PATH)
    IF (NOT path STREQUAL ${CMAKE_SOURCE_DIR})
      GET_FILENAME_COMPONENT(directory ${path} NAME)
      STRING(TOUPPER ${directory} DIRECTORY)
      SET( ${directory}_environment ${${directory}_environment} ${DIRECTORY}ROOT=${path})
      GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${directory} "${${directory}_environment}")
    ENDIF()
  ENDFOREACH()
ENDFUNCTION()


################################
#---GAUDI_BUILD_PACKAGE_SETUP
################################
FUNCTION( GAUDI_BUILD_PACKAGE_SETUP setup package envlist )
  IF ( NOT setup )
    SET( setup ${CMAKE_INSTALL_PREFIX}/${package}_setup.csh )
    FILE(WRITE  ${setup} "# Package ${package} setup file\n")
  ELSE()
    FILE(APPEND  ${setup} "\n# Package ${package} setup file\n")
  ENDIF()
  FOREACH( env ${envlist} )
    #MESSAGE("env = ${env}")
    IF(env MATCHES ".*[+]=.*")
      STRING(REGEX REPLACE "([^=+]+)[+]=.*" "\\1" var ${env})
      STRING(REGEX REPLACE ".*[+]=(.+)" "\\1"  val ${env})
      FILE(APPEND ${setup} "setenv ${var} ${val}:\${${var}}\n")
    ELSEIF ( env MATCHES ".*=.*")
      STRING(REGEX REPLACE "([^=+]+)=.*" "\\1" var ${env})
      STRING(REGEX REPLACE ".*=(.+)" "\\1"  val ${env})
      FILE(APPEND ${setup} "setenv ${var} ${val}\n")
   ENDIF() 
  ENDFOREACH()
ENDFUNCTION()
