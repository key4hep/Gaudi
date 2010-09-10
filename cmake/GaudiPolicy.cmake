#---------------------------------------------------------------------------------------------------
#  GaudiPolicy.cmake   (General flags and options for GAUDI projects -------------------------------
#---------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.4.6)
cmake_policy(SET CMP0003 NEW) # See "cmake --help-policy CMP0003" for more details
cmake_policy(SET CMP0011 NEW) # See "cmake --help-policy CMP0011" for more details
cmake_policy(SET CMP0009 NEW) # See "cmake --help-policy CMP0009" for more details

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/InstallArea)
set(lib $ENV{CMTCONFIG}/lib)
set(bin $ENV{CMTCONFIG}/bin)

set(CMAKE_VERBOSE_MAKEFILES ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_COMPILER g++)


# Compilation Flags
#-->set(CMAKE_CXX_FLAGS "-Dunix -pipe -ansi -Wall -Wextra -pthread  -Wno-deprecated -Wwrite-strings -Wpointer-arith -Woverloaded-virtual -Wno-long-long")


set(CMAKE_CXX_FLAGS "-Dunix -pipe -ansi -Wall -Wextra -pthread  -Wno-deprecated -Wwrite-strings -Wpointer-arith -Wno-long-long")
if (CMAKE_SYSTEM_NAME MATCHES Linux) 
  set(CMAKE_CXX_FLAGS "-Dlinux ${CMAKE_CXX_FLAGS}")
endif()

add_definitions(-D_GNU_SOURCE -DGAUDI_V20_COMPAT)

# Link shared flags
if (CMAKE_SYSTEM_NAME MATCHES Linux) 
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000")
  set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000")
endif()

if(APPLE)
   set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -flat_namespace -single_module -undefined dynamic_lookup")
   set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -flat_namespace -single_module -undefined dynamic_lookup")
endif()

if(APPLE)
  set(ld_library_path DYLD_LIBRARY_PATH)
else()
  set(ld_library_path LD_LIBRARY_PATH)
endif() 

find_program(python_cmd python)
if(CMAKE_PROJECT_NAME STREQUAL GAUDI)
  set(merge_rootmap_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/merge_files.py)
  set(merge_conf_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/merge_files.py)
  set(genconf_cmd ${CMAKE_BINARY_DIR}/GaudiKernel/genconf.exe)
  set(versheader_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/createProjVersHeader.py)
else()
  set(merge_rootmap_cmd ${python_cmd}  ${GAUDI_installation}/GaudiPolicy/scripts/merge_files.py)
  set(merge_conf_cmd ${python_cmd}  ${GAUDI_installation}/GaudiPolicy/scripts/merge_files.py)
  set(genconf_cmd ${GAUDI_binaryarea}/bin/genconf.exe)
  set(versheader_cmd ${python_cmd} ${GAUDI_installation}/GaudiPolicy/scripts/createProjVersHeader.py)
  set(GAUDI_SOURCE_DIR ${GAUDI_installation})
endif()


#---------------------------------------------------------------------------------------------------
#---REFLEX_GENERATE_DICTIONARY
#---------------------------------------------------------------------------------------------------
macro(REFLEX_GENERATE_DICTIONARY dictionary _headerfiles _selectionfile)  
  find_package(GCCXML)
  find_package(ROOT)

  set(options ${ARGN})
  if( IS_ABSOLUTE ${_selectionfile}) 
   set( selectionfile ${_selectionfile})
  else() 
   set( selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${_selectionfile}) 
  endif()
  if( IS_ABSOLUTE ${_headerfiles}) 
    set( headerfiles ${_headerfiles})
  else()
    set( headerfiles ${CMAKE_CURRENT_SOURCE_DIR}/${_headerfiles})
  endif()
 
  set(gensrcdict ${dictionary}_dict.cpp)
  #set(gccxmlopts "--gccxmlopt=\'--gccxml-cxxflags -m64 \'")
  set(gccxmlopts)
  set(rootmapname ${dictionary}Dict.rootmap)
  set(rootmapopts --rootmap=${rootmapname} --rootmap-lib=lib${dictionary}Dict)

  set(include_dirs -I${CMAKE_CURRENT_SOURCE_DIR})
  get_directory_property(_incdirs INCLUDE_DIRECTORIES)
  foreach( d ${_incdirs})    
   set(include_dirs ${include_dirs} -I${d})
  endforeach()

  get_directory_property(_defs COMPILE_DEFINITIONS)
  foreach( d ${_defs})    
   set(definitions ${definitions} -D${d})
  endforeach()
 
  if (CMAKE_SYSTEM_NAME MATCHES Linux)
    add_custom_command(
      OUTPUT ${gensrcdict} ${rootmapname}     
      COMMAND ${ROOT_genreflex_cmd}       
      ARGS ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
           --gccxmlpath=${GCCXML_home}/bin ${options} ${include_dirs} ${definitions}
      DEPENDS ${headerfiles} ${selectionfile})  
  else() 
    add_custom_command(
      OUTPUT ${gensrcdict} ${rootmapname}      
      COMMAND ${ROOT_genreflex_cmd}       
      ARGS ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
           --gccxmlpath=${GCCXML_home}/bin ${include_dirs} ${definitions}
      DEPENDS ${headerfiles} ${selectionfile})     
  endif()
endmacro()

#---------------------------------------------------------------------------------------------------
#---REFLEX_BUILD_DICTIONARY
#---------------------------------------------------------------------------------------------------
function(REFLEX_BUILD_DICTIONARY dictionary headerfiles selectionfile )  
  REFLEX_GENERATE_DICTIONARY(${dictionary} ${headerfiles} ${selectionfile})
  add_library(${dictionary}Dict MODULE ${gensrcdict})
  target_link_libraries(${dictionary}Dict ${ARGN} Reflex)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${dictionary}Dict LIBRARY DESTINATION ${lib})
  set(mergedRootMap ${CMAKE_INSTALL_PREFIX}/${lib}/${CMAKE_PROJECT_NAME}Dict.rootmap)
  set(srcRootMap ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname})
  install(CODE "EXECUTE_PROCESS(COMMAND ${merge_rootmap_cmd} --do-merge --input-file ${srcRootMap} --merged-file ${mergedRootMap})")
endfunction()

#---------------------------------------------------------------------------------------------------
#---SET_RUNTIME_PATH
#---------------------------------------------------------------------------------------------------
macro( SET_RUNTIME_PATH var)
  set( runtime_dirs ${GAUDI_LIBRARY_DIRS})
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  get_property(found_projects GLOBAL PROPERTY PROJECTS_FOUND)
  foreach( package ${found_projects} ${found_packages} )
     foreach( env ${${package}_environment})
         if(env MATCHES "LD_LIBRARY_PATH[+]=.*")
            string(REGEX REPLACE "LD_LIBRARY_PATH[+]=(.+)" "\\1"  val ${env})
            set(runtime_dirs ${val} ${runtime_dirs})
         endif()
     endforeach()
  endforeach()
  foreach(p ${runtime_dirs})
    if( ${var} )
      set(${var} ${${var}}:${p})
    else()
      set(${var} ${p})
    endif()
  endforeach()
endmacro()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_ROOTMAP
#---------------------------------------------------------------------------------------------------
function(GAUDI_GENERATE_ROOTMAP library)
  find_package(ROOT)
  set(rootmapfile ${library}.rootmap)
  set(fulllibname lib${library}.so)
  SET_RUNTIME_PATH(ld_path)
  add_custom_command( OUTPUT ${rootmapfile}
                      COMMAND ${ld_library_path}=.:${ld_path}:$ENV{${ld_library_path}} ${ROOT_genmap_cmd} -i ${fulllibname} -o ${rootmapfile} 
                      DEPENDS ${library} )
  add_custom_target( ${library}Rootmap ALL DEPENDS  ${rootmapfile})
  #----Installation details-------------------------------------------------------
  set(mergedRootMap ${CMAKE_INSTALL_PREFIX}/${lib}/${CMAKE_PROJECT_NAME}.rootmap)
  set(srcRootMap ${CMAKE_CURRENT_BINARY_DIR}/${library}.rootmap)
  install(CODE "EXECUTE_PROCESS(COMMAND ${merge_rootmap_cmd} --do-merge --input-file ${srcRootMap} --merged-file ${mergedRootMap})")
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_CONFIGURATION
#---------------------------------------------------------------------------------------------------
function(GAUDI_GENERATE_CONFIGURATION library)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(library_preload)  # TODO....
  set(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})
  file(MAKE_DIRECTORY ${outdir})
  set(confModuleName GaudiKernel.Proxy)
  set(confDefaultName Configurable.DefaultName)
  set(confAlgorithm ConfigurableAlgorithm)
  set(confAlgTool ConfigurableAlgTool)
  set(confAuditor ConfigurableAuditor)
  set(confService ConfigurableService)
  SET_RUNTIME_PATH(ld_path)
  if( TARGET GaudiSvc)
	  set(GaudiSvc_dependency GaudiSvc)
  endif()
  add_custom_command( 
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
  add_custom_target( ${library}Conf ALL DEPENDS  ${outdir}/${library}_confDb.py )
  #----Installation details-------------------------------------------------------
  set(mergedConf ${CMAKE_INSTALL_PREFIX}/python/${CMAKE_PROJECT_NAME}_merged_confDb.py)
  set(srcConf ${outdir}/${library}_confDb.py)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConf/ DESTINATION python PATTERN "*.stamp" EXCLUDE PATTERN "*.pyc" EXCLUDE )
  install(CODE "EXECUTE_PROCESS(COMMAND ${merge_conf_cmd} --do-merge --input-file ${srcConf} --merged-file ${mergedConf})")
  GAUDI_INSTALL_PYTHON_INIT()  
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_LINKER_LIBRARY
#---------------------------------------------------------------------------------------------------
function(GAUDI_LINKER_LIBRARY library sources )
  set(lib_srcs)
  foreach( fp ${sources})
    file(GLOB files src/${fp})
    if(files) 
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${library} ${lib_srcs})
  set_target_properties(${library} PROPERTIES COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY)
  target_link_libraries(${library} ${ARGN})
  if(TARGET ${library}Obj2doth)
    add_dependencies( ${library} ${library}Obj2doth) 
  endif()
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} LIBRARY DESTINATION  ${lib})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_COMPONENT_LIBRARY
#---------------------------------------------------------------------------------------------------
function(GAUDI_COMPONENT_LIBRARY library sources )
  set(lib_srcs)
  foreach( fp ${sources})  
    file(GLOB files src/${fp})
    if(files) 
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${library} MODULE ${lib_srcs})
  GAUDI_GENERATE_ROOTMAP(${library})
  GAUDI_GENERATE_CONFIGURATION(${library})
  target_link_libraries(${library} Reflex ${ARGN})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} LIBRARY DESTINATION ${lib})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_EXECUTABLE
#---------------------------------------------------------------------------------------------------
function(GAUDI_EXECUTABLE executable sources)
  set(exe_srcs)
  foreach( fp ${sources})  
    file(GLOB files src/${fp})
    if(files) 
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  add_executable( ${executable} ${exe_srcs})
  target_link_libraries(${executable} ${ARGN} )
  set_target_properties(${executable} PROPERTIES SUFFIX .exe)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_TEST
#---------------------------------------------------------------------------------------------------
function(GAUDI_TEST executable sources)
  set(exe_srcs)
  foreach( fp ${sources})  
    file(GLOB files src/${fp})
    if(files) 
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  add_executable( ${executable} ${exe_srcs})
  target_link_libraries(${executable} ${ARGN} )
  #----Installation details-------------------------------------------------------
  set_target_properties(${executable} PROPERTIES SUFFIX .exe)
  install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_HEADERS
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_HEADERS)
  if( ARGN )
    set( dirs ${ARGN} )
  else()
    get_filename_component(dirs ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  endif()
  foreach( inc ${dirs})  
    install(DIRECTORY ${inc} DESTINATION include PATTERN ".svn" EXCLUDE )
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_MODULES
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_MODULES)  
  install(DIRECTORY python/ DESTINATION python 
          PATTERN ".svn" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
  GAUDI_INSTALL_PYTHON_INIT()  
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_INIT
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_INIT)    
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(CODE "if (NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/python/${package}/__init__.py\")
                  file(INSTALL DESTINATION \"${CMAKE_INSTALL_PREFIX}/python/${package}\"
                               TYPE FILE 
                               FILES \"${GAUDI_SOURCE_DIR}/GaudiPolicy/cmt/fragments/__init__.py\"  )
                endif()" )
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_SCRIPTS
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_SCRIPTS)
  install(DIRECTORY scripts/ DESTINATION scripts 
          FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ 
          PATTERN ".svn" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_PROJECT_VERSION_HEADER
#---------------------------------------------------------------------------------------------------
function( GAUDI_PROJECT_VERSION_HEADER )
  set(project ${CMAKE_PROJECT_NAME})
  set(version ${${CMAKE_PROJECT_NAME}_VERSION})
  set(ProjectVersionHeader_output  ${CMAKE_INSTALL_PREFIX}/include/${project}_VERSION.h)
  add_custom_target( ${project}VersionHeader ALL
                     ${versheader_cmd} ${project} ${version} ${ProjectVersionHeader_output} )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_USE_PACKAGE
#---------------------------------------------------------------------------------------------------
macro( GAUDI_USE_PACKAGE package )
  if( EXISTS ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt)
    include_directories( ${CMAKE_SOURCE_DIR}/${package} ) 
    file(READ ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt file_contents)
    string( REGEX MATCHALL "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])+" vars ${file_contents})
    foreach( var ${vars})
      string(REGEX REPLACE "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])" "\\1" p ${var})
      GAUDI_USE_PACKAGE(${p})
    endforeach()
  else()
    find_package(${package})
    GET_PROPERTY(parent DIRECTORY PROPERTY PARENT_DIRECTORY)
    if(parent)
      set(${package}_environment  ${${package}_environment} PARENT_SCOPE)
    else()
      set(${package}_environment  ${${package}_environment} )
    endif()
    include_directories( ${${package}_INCLUDE_DIRS} ) 
    link_directories( ${${package}_LIBRARY_DIRS} ) 
  endif()
endmacro()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_SETUP
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_SETUP )
  GAUDI_BUILD_PROJECT_SETUP()
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  message("Found packages = ${found_packages}")
  foreach( package ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( "" ${package} "${${package}_environment}")
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_PROJECT_SETUP
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PROJECT_SETUP )
  set( setup  ${CMAKE_INSTALL_PREFIX}/setup.csh )
  file(WRITE  ${setup} "# ${CMAKE_PROJECT_NAME} Setup file\n")
  file(APPEND ${setup} "setenv PATH  ${CMAKE_INSTALL_PREFIX}/${bin}:${CMAKE_INSTALL_PREFIX}/scripts:\${PATH}\n")
  file(APPEND ${setup} "setenv LD_LIBRARY_PATH  ${CMAKE_INSTALL_PREFIX}/${lib}:\${LD_LIBRARY_PATH}\n")
  file(APPEND ${setup} "setenv PYTHONPATH  ${CMAKE_INSTALL_PREFIX}/python:\${PYTHONPATH}\n")
  file(APPEND ${setup} "setenv JOBOPTSEARCHPATH  ${CMAKE_SOURCE_DIR}/GaudiPoolDb/options:${CMAKE_SOURCE_DIR}/GaudiExamples/options\n")

  #----Get the setup fro each external package
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  get_property(found_projects GLOBAL PROPERTY PROJECTS_FOUND)
  #MESSAGE("Found packages = ${found_packages}")
  foreach( package ${found_projects} ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${package} "${${package}_environment}")
  endforeach()

  #---Get the setup for each package (directory)
  file(APPEND  ${setup} "\n# Standard variables for each package\n")
  file(GLOB_RECURSE cmakelist_files  ${CMAKE_SOURCE_DIR} CMakeLists.txt)
  foreach( file ${cmakelist_files} )
    GET_FILENAME_COMPONENT(path ${file} PATH)
    if (NOT path STREQUAL ${CMAKE_SOURCE_DIR})
      GET_FILENAME_COMPONENT(directory ${path} NAME)
      string(TOUPPER ${directory} DIRECTORY)
      set( ${directory}_environment ${${directory}_environment} ${DIRECTORY}ROOT=${path})
      GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${directory} "${${directory}_environment}")
    endif()
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_PACKAGE_SETUP
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PACKAGE_SETUP setup package envlist )
  if ( NOT setup )
    set( setup ${CMAKE_INSTALL_PREFIX}/${package}_setup.csh )
    file(WRITE  ${setup} "# Package ${package} setup file\n")
  else()
    file(APPEND  ${setup} "\n# Package ${package} setup file\n")
  endif()
  foreach( env ${envlist} )
    #MESSAGE("env = ${env}")
    if(env MATCHES ".*[+]=.*")
      string(REGEX REPLACE "([^=+]+)[+]=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*[+]=(.+)" "\\1"  val ${env})
      file(APPEND ${setup} "setenv ${var} ${val}:\${${var}}\n")
    elseif ( env MATCHES ".*=.*")
      string(REGEX REPLACE "([^=+]+)=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*=(.+)" "\\1"  val ${env})
      file(APPEND ${setup} "setenv ${var} ${val}\n")
   endif() 
  endforeach()
endfunction()
