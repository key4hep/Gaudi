# - GaudiProject
# Define the macros used by Gaudi-based projects, namely:
#  GAUDI_PROJECT(project version) : declare a project with it's version number
#  GAUDI_USE_PROJECT(project version) : declare the dependency on another project
#
# Authors: Pere Mato, Marco Clemencic

cmake_minimum_required(VERSION 2.4.6)
cmake_policy(SET CMP0003 NEW) # See "cmake --help-policy CMP0003" for more details
cmake_policy(SET CMP0011 NEW) # See "cmake --help-policy CMP0011" for more details
cmake_policy(SET CMP0009 NEW) # See "cmake --help-policy CMP0009" for more details

# Add the directory containing this file to the modules search path
set(CMAKE_MODULE_PATH ${GaudiProject_DIR} ${CMAKE_MODULE_PATH})

if(DEFINED LCG_system)
  # We are using the LCG toolchain, so enable special configuration
  get_filename_component(LCG_TOOLCHAIN_PATH ${CMAKE_TOOLCHAIN_FILE} PATH)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LCG_TOOLCHAIN_PATH})
  # Define the versions and search paths
  if(DEFINED LCG_version)
    include(LCG_${LCG_version}/Configuration)
    # FIXME: temporary
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LCG_TOOLCHAIN_PATH}/LCG_${LCG_version})
    #include(Configuration)
  else()
    message(FATAL_ERROR "LCG required but LCG_version is not defined")
  endif()
elseif(DEFINED ENV{CMTCONFIG} OR DEFINED ENV{CMAKECONFIG})
  message(FATAL_ERROR "You must use the LCG CMAKE_TOOLCHAIN_FILE if you have CMTCONFIG or CMAKECONFIG set.")
endif()

#-------------------------------------------------------------------------------
# Basic configuration
#-------------------------------------------------------------------------------
set(CMAKE_VERBOSE_MAKEFILES OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_SKIP_BUILD_RPATH TRUE)
#set(CMAKE_CXX_COMPILER g++)

#-------------------------------------------------------------------------------
# Platform handling
#-------------------------------------------------------------------------------
#function(GET_LCG_TAG sysvar platfvar)
#  if(CMAKE_SYSTEM_PROCESSOR
#  if(CMAKE_BUILD_TYPE STREQUAL Debug)
#
#  else()
#  endif(statement)
#
#endfunction()

#-------------------------------------------------------------------------------
# Platform transparency
#-------------------------------------------------------------------------------
if(WIN32)
  set(ld_library_path PATH)
elseif(APPLE)
  set(ld_library_path DYLD_LIBRARY_PATH)
else()
  set(ld_library_path LD_LIBRARY_PATH)
endif()

set(lib lib)
set(bin bin)

if(WIN32)
  set(ssuffix .bat)
  set(scomment rem)
  set(libprefix "")
else()
  set(ssuffix .csh)
  set(scomment \#)
  set(libprefix lib)
endif()


#---------------------------------------------------------------------------------------------------
# Programs and utilities needed for the build
#---------------------------------------------------------------------------------------------------
include(CMakeMacroParseArguments)

find_package(PythonInterp)

#--- commands required to build cached variable
# (python scripts are located as such but run through python)
set(hints ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts ${CMAKE_SOURCE_DIR}/GaudiKernel/scripts)

find_program(env_cmd env.py HINTS ${hints})
set(env_cmd ${PYTHON_EXECUTABLE} ${env_cmd})

find_program(merge_cmd merge_files.py HINTS ${hints})
set(merge_cmd ${PYTHON_EXECUTABLE} ${merge_cmd} --no-stamp)

find_program(versheader_cmd createProjVersHeader.py HINTS ${hints})
set(versheader_cmd ${PYTHON_EXECUTABLE} ${versheader_cmd})

find_program(genconfuser_cmd genconfuser.py HINTS ${hints})
set(genconfuser_cmd ${PYTHON_EXECUTABLE} ${genconfuser_cmd})

find_program(zippythondir_cmd ZipPythonDir.py HINTS ${hints})
set(zippythondir_cmd ${PYTHON_EXECUTABLE} ${zippythondir_cmd})

#---------------------------------------------------------------------------------------------------
#---GAUDI_PROJECT(project version)
#---------------------------------------------------------------------------------------------------
macro(GAUDI_PROJECT project_name version)
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
  string(TOUPPER ${project_name} project)
  project(${project})
  set(CMAKE_PROJECT_NAME ${project}) #----For some reason this is not set by calling 'project()'

  #--- Define the version of the project - can be used to generate sources,
  set(${project}_VERSION ${version})
  if( ${version} MATCHES "[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)")
    string(REGEX REPLACE "[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)" "\\1"  major ${version})
    string(REGEX REPLACE "[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)" "\\2"  minor ${version})
    string(REGEX REPLACE "[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)" "\\3"  patch ${version})
  elseif(${version} MATCHES "[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)")
    string(REGEX REPLACE "[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)" "\\1"  major ${version})
    string(REGEX REPLACE "[a-zA-Z]+([0-9]+)[a-zA-Z]+([0-9]+)" "\\2"  minor ${version})
    set( patch 0)
  endif()
  set(${project}_VERSION_MAJOR ${major})
  set(${project}_VERSION_MINOR ${minor})
  set(${project}_VERSION_PATCH ${patch})

  #--- Project Options and Global settings----------------------------------------------------------
  option(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON)
  option(BUILD_TESTS "Set to ON to build the tests (libraries and executables)" OFF)
  option(HIDE_WARNINGS "Turn on or off options that are used to hide warning messages" ON)
  #-------------------------------------------------------------------------------------------------

  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/InstallArea/${BINARY_TAG} CACHE PATH
      "Install path prefix, prepended onto install directories." FORCE )
  endif()

  if(NOT BUILD_OUTPUT_PREFIX)
    set(BUILD_OUTPUT_PREFIX ${CMAKE_BINARY_DIR}/.build CACHE STRING
           "Base directory for generated files" FORCE)
  endif()
  if(NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${BUILD_OUTPUT_PREFIX}/bin CACHE STRING
	   "Single build output directory for all executables" FORCE)
  endif()
  if(NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${BUILD_OUTPUT_PREFIX}/lib CACHE STRING
	   "Single build output directory for all libraries" FORCE)
  endif()
  if(NOT PYTHON_OUTPUT_PATH)
    set(PYTHON_OUTPUT_PATH ${BUILD_OUTPUT_PREFIX}/python CACHE STRING
	   "Single build output directory for all python files" FORCE)
  endif()

  if(BUILD_TESTS)
    enable_testing()
  endif()

  # FIXME: external tools need to be found independently of the project
  if(CMAKE_PROJECT_NAME STREQUAL GAUDI)
    set(genconf_cmd ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/genconf.exe)
    set(genwindef_cmd ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/genwindef.exe)
    set(gaudirun ${CMAKE_SOURCE_DIR}/Gaudi/scripts/gaudirun.py)
  else()
    set(genconf_cmd ${GAUDI_binaryarea}/bin/genconf.exe)
    set(genwindef_cmd ${GAUDI_binaryarea}/bin/genwindef.exe)
    set(GAUDI_SOURCE_DIR ${GAUDI_installation})
    set(gaudirun ${GAUDI_installarea}/scripts/gaudirun.py)
  endif()

  #--- Project Installations------------------------------------------------------------------------
  install(DIRECTORY cmake/ DESTINATION cmake
                           FILES_MATCHING PATTERN "*.cmake"
                           PATTERN ".svn" EXCLUDE )
  install(PROGRAMS cmake/testwrap.sh cmake/testwrap.csh cmake/testwrap.bat cmake/genCMake.py cmake/env.py DESTINATION scripts)

  #--- Global actions for the project
  INCLUDE(GaudiContrib)
  INCLUDE(GaudiBuildFlags)

  message(STATUS "Looking for local directories...")
  GAUDI_GET_PACKAGES(packages)
  message(STATUS "Found:")
  foreach(package ${packages})
    message(STATUS "  ${package}")
    # FIXME: this ensures that all the packages directories are added to the include paths,
    #        but it doesn't enforce that only the actually exported headers are visible.
    include_directories(${package})
  endforeach()

  #GAUDI_SORT_PACKAGES(packages ${packages})
  foreach(package ${packages})
    message(STATUS "Adding directory ${package}")
    add_subdirectory(${package})
  endforeach()

  GAUDI_PROJECT_VERSION_HEADER()
  GAUDI_BUILD_PROJECT_SETUP()
  GAUDI_MERGE_TARGET(ConfDB python ${CMAKE_PROJECT_NAME}_merged_confDb.py)
  GAUDI_MERGE_TARGET(Rootmap lib ${CMAKE_PROJECT_NAME}.rootmap)
  GAUDI_MERGE_TARGET(DictRootmap lib ${CMAKE_PROJECT_NAME}Dict.rootmap)


  #GAUDI_USE_PACKAGE(QMtest)
  #GAUDI_USE_PACKAGE(pytools)
  #GAUDI_USE_PACKAGE(RELAX)
  #SET( QMtest_environment ${QMtest_environment} QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes )

  #--- CPack configuration
  set(CPACK_PACKAGE_NAME ${project_name})
  foreach(t MAJOR MINOR PATCH)
    set(CPACK_PACKAGE_VERSION_${t} ${${project}_VERSION_${t}})
  endforeach()
  set(CPACK_SYSTEM_NAME ${BINARY_TAG})

  set(CPACK_GENERATOR TGZ)

  include(CPack)

endmacro()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BINARY_TAG()
#---------------------------------------------------------------------------------------------------
function(GAUDI_BINARY_TAG)
  message("CMAKE_SYSTEM --> ${CMAKE_SYSTEM}")
  execute_process(COMMAND uname -i OUTPUT_VARIABLE arch)
  message("arch --> ${arch}")
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_FIND_PROJECT(project version)
#---------------------------------------------------------------------------------------------------
macro( GAUDI_FIND_PROJECT project version)
  file(TO_CMAKE_PATH "$ENV{CMAKEPROJECTPATH}" projectpath)
  foreach( path ${projectpath})
    if(EXISTS ${path}/${project})
      if(${version} STREQUAL "*")
        file(GLOB installations ${path}/${project}/${project}_* )
        foreach( installation ${installations})
          if(EXISTS ${installation}/InstallArea/${BINARY_TAG}/cmake )
            set(${project}_installation ${installation})
            break()
          endif()
        endforeach()
      else()
        if(EXISTS  ${path}/${project}/${project}_${version}/InstallArea/${BINARY_TAG}/cmake)
          set(${project}_installation ${path}/${project}/${project}_${version})
          break()
        endif()
      endif()
    endif()
    if(${project}_found)
      break()
    endif()
  endforeach()

  if( ${project}_installation )
    message("Found project ${project} with version ${version} at ${${project}_installation}")
    set(${project}_found 1)
    set(${project}_installarea ${${project}_installation}/InstallArea)
    set(${project}_binaryarea  ${${project}_installation}/InstallArea/${BINARY_TAG})
  else()
    message(ERROR " Project ${project} with version ${version} not found!!")
  endif()
endmacro()

#---------------------------------------------------------------------------------------------------
#---GAUDI_USE_PROJECT(project version)
#---------------------------------------------------------------------------------------------------
macro( GAUDI_USE_PROJECT project version )
  if( NOT ${project}_used )
    GAUDI_FIND_PROJECT(${project} ${version})
    if( ${project}_installation )
      #------Set the list of variables to make a effective 'use' of the project-----
      get_property(projects GLOBAL PROPERTY PROJECTS_FOUND)
      set_property(GLOBAL PROPERTY PROJECTS_FOUND ${projects} ${project})
      get_property(projects GLOBAL PROPERTY PROJECTS_FOUND)
      set(CMAKE_MODULE_PATH ${${project}_binaryarea}/cmake ${CMAKE_MODULE_PATH})
      include_directories( ${${project}_binaryarea}/include )
      link_directories( ${${project}_binaryarea}/lib )
      set(${project}_environment ${ld_library_path}+=${${project}_binaryarea}/lib
                                 PATH+=${${project}_binaryarea}/bin
                                 PATH+=${${project}_binaryarea}/scripts
                                 PYTHONPATH+=${${project}_binaryarea}/python )
      include(${project}Exports)
      set(${project}_used 1)
      #------------------------------------------------------------------------------
      if( EXISTS ${${project}_installation}/CMakeLists.txt)
        file(READ ${${project}_installation}/CMakeLists.txt file_contents)
        string( REGEX MATCHALL "GAUDI_USE_PROJECT[ ]*[(][ ]*([^)])+" vars ${file_contents})
        foreach( var ${vars})
          string(REGEX REPLACE "GAUDI_USE_PROJECT[ ]*[(][ ]*([^)])" "\\1" p ${var})
          separate_arguments(p)
          GAUDI_USE_PROJECT(${p})
        endforeach()
      endif()
    endif()
  endif()
endmacro()

#---------------------------------------------------------------------------------------------------
#---GAUDI_SORT_PACKAGES
#---------------------------------------------------------------------------------------------------
macro(__visit__ _p)
  if( NOT __${_p}_visited__)
    set(__${_p}_visited__ 1)
    #---list all dependent packages-----
    if( EXISTS ${CMAKE_SOURCE_DIR}/${_p}/CMakeLists.txt)
      file(READ ${CMAKE_SOURCE_DIR}/${_p}/CMakeLists.txt file_contents)
      string( REGEX MATCHALL "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])+" vars ${file_contents})
      foreach( var ${vars})
        string(REGEX REPLACE "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])" "\\1" __p ${var})
        __visit__(${__p})
      endforeach()
      set(out_packages ${out_packages} ${_p})
    endif()
  endif()
endmacro()

function(GAUDI_SORT_PACKAGES var)
  set(out_packages)
  set(in_packages ${ARGN})
  foreach(p ${in_packages})
    __visit__(${p})
  endforeach()
  set(${var} ${out_packages} PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GET_PACKAGES
#---------------------------------------------------------------------------------------------------
function( GAUDI_GET_PACKAGES var)
  set( packages )
  file(GLOB_RECURSE cmakelist_files  ${CMAKE_SOURCE_DIR} CMakeLists.txt)
  foreach( file ${cmakelist_files} )
    get_filename_component(path ${file} PATH)
    if( NOT path STREQUAL ${CMAKE_SOURCE_DIR})
      string(REPLACE ${CMAKE_SOURCE_DIR}/ "" package ${path})
      SET(packages ${packages} ${package})
    endif()
  endforeach()
  set(${var} ${packages} PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_MERGE_TARGET
#---------------------------------------------------------------------------------------------------
# Create a MergedXXX target that takes input files and dependencies from
# properties of the packages
function(GAUDI_MERGE_TARGET tgt dest filename)
  # Check if one of the packages produces files for this merge target
  get_property(needed GLOBAL PROPERTY Merged${tgt}_SOURCES SET)
  if(needed)
    # get the list of parts to merge
    get_property(parts GLOBAL PROPERTY Merged${tgt}_SOURCES)
    # create the targets
    set(output ${BUILD_OUTPUT_PREFIX}/${dest}/${filename})
    add_custom_command(OUTPUT ${output}
                       COMMAND ${merge_cmd} ${parts} ${output}
                       DEPENDS ${parts})
    add_custom_target(Merged${tgt} ALL DEPENDS ${output})
    # prepare the high level dependencies
    get_property(deps GLOBAL PROPERTY Merged${tgt}_DEPENDS)
    add_dependencies(Merged${tgt} ${deps})
    # install rule for the merged DB
    install(FILES ${output} DESTINATION ${dest})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_CONFIGURABLES( library )
#---------------------------------------------------------------------------------------------------
function(GAUDI_GENERATE_CONFIGURABLES library)
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
  SET_RUNTIME_PATH(path ${ld_library_path})
  add_custom_command(
    OUTPUT ${outdir}/${library}_confDb.py ${outdir}/${library}Conf.py ${outdir}/__init__.py
		COMMAND ${env_cmd}
                  -p ${ld_library_path}=${path}
                  -p ${ld_library_path}=.
		        ${genconf_cmd} ${library_preload} -o ${outdir} -p ${package}
				--configurable-module=${confModuleName}
				--configurable-default-name=${confDefaultName}
				--configurable-algorithm=${confAlgorithm}
				--configurable-algtool=${confAlgTool}
				--configurable-auditor=${confAuditor}
				--configurable-service=${confService}
				-i lib${library}.so
		DEPENDS ${library} )
  add_custom_target( ${library}Conf ALL DEPENDS  ${outdir}/${library}_confDb.py )
  # Add dependencies on GaudiSvc and the genconf executable if they have to be built in the current project
  add_dependencies(${library}Conf genconf GaudiSvc)
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedConfDB_SOURCES ${outdir}/${library}_confDb.py)
  set_property(GLOBAL APPEND PROPERTY MergedConfDB_DEPENDS ${library}Conf)
  #----Installation details-------------------------------------------------------
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConf/
          DESTINATION python
          FILES_MATCHING PATTERN "*.py")
  GAUDI_INSTALL_PYTHON_INIT()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_CONFUSERDB([DEPENDS target1 target2])
#---------------------------------------------------------------------------------------------------
define_property(DIRECTORY
                PROPERTY CONFIGURABLE_USER_MODULES
                BRIEF_DOCS "ConfigurableUser modules"
                FULL_DOCS "List of Python modules containing ConfigurableUser specializations (default <package>/Config, 'None' to disable)." )
# Generate entries in the configurables database for ConfigurableUser specializations.
# By default, the python module supposed to contain ConfigurableUser's is <package>.Config,
# but different (or more) modules can be specified with the directory property
# CONFIGURABLE_USER_MODULES. If that property is set to None, there will be no
# search for ConfigurableUser's.
function(GAUDI_GENERATE_CONFUSERDB)
  # deduce the name of the package
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  get_directory_property(modules CONFIGURABLE_USER_MODULES)
  if( NOT (modules STREQUAL "None") ) # ConfUser enabled
    set(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})
    # get the optional dependencies from argument and properties
    PARSE_ARGUMENTS(ARG "DEPENDS" "" ${arguments})
    get_directory_property(PROPERTY_DEPENDS CONFIGURABLE_USER_DEPENDS)
    SET_RUNTIME_PATH(path PYTHONPATH)
    # TODO: this re-runs the genconfuser every time, because we cannot define the right dependencies
    add_custom_target(${package}ConfUserDB ALL
                      DEPENDS ${outdir}/${package}_user_confDb.py)
    if(${ARG_DEPENDS} ${PROPERTY_DEPENDS})
      add_dependencies(${package}ConfUserDB ${ARG_DEPENDS} ${PROPERTY_DEPENDS})
    endif()
    add_custom_command(OUTPUT ${outdir}/${package}_user_confDb.py
		COMMAND ${env_cmd}
                  -p PYTHONPATH=${path}
                  -p PYTHONPATH=${CMAKE_SOURCE_DIR}/GaudiKernel/python
                ${genconfuser_cmd}
		          -r ${CMAKE_CURRENT_SOURCE_DIR}/python
		          -o ${outdir}/${package}_user_confDb.py
		          ${package} ${modules})
    set_property(GLOBAL APPEND PROPERTY MergedConfDB_SOURCES ${outdir}/${package}_user_confDb.py)
    set_property(GLOBAL APPEND PROPERTY MergedConfDB_DEPENDS ${package}ConfUserDB)
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_LINKER_LIBRARY( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_LINKER_LIBRARY library)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  if(WIN32)
	add_library( ${library}-arc STATIC EXCLUDE_FROM_ALL ${lib_srcs})
    set_target_properties(${library}-arc PROPERTIES COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY )
    add_custom_command(
      OUTPUT ${library}.def
	  COMMAND ${genwindef_cmd} -o ${library}.def -l ${library} ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${library}-arc.lib
	  DEPENDS ${library}-arc genwindef)
	#---Needed to create a dummy source file to please Windows IDE builds with the manifest
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/${library}.cpp "// empty file\n" )
    add_library( ${library} SHARED ${library}.cpp ${library}.def)
    target_link_libraries(${library} ${library}-arc ${ARG_LIBRARIES})
    set_target_properties(${library} PROPERTIES LINK_INTERFACE_LIBRARIES "${ARG_LIBRARIES}" )
  else()
    add_library( ${library} ${lib_srcs})
    set_target_properties(${library} PROPERTIES COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY )
    target_link_libraries(${library} ${ARG_LIBRARIES})
  endif()
  if(TARGET ${library}Obj2doth)
    add_dependencies( ${library} ${library}Obj2doth)
  endif()
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION  ${lib})
  install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_COMPONENT_LIBRARY( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_COMPONENT_LIBRARY library)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${library} MODULE ${lib_srcs})
  GAUDI_GENERATE_ROOTMAP(${library})
  GAUDI_GENERATE_CONFIGURABLES(${library})
  target_link_libraries(${library} ${ROOT_Reflex_LIBRARY} ${ARG_LIBRARIES})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} LIBRARY DESTINATION ${lib})
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_PYTHON_MODULE( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_PYTHON_MODULE module)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${module} MODULE ${lib_srcs})
  if(win32)
    set_target_properties( ${module} PROPERTIES SUFFIX .pyd PREFIX "")
  else()
    set_target_properties( ${module} PROPERTIES SUFFIX .so PREFIX "")
  endif()
  target_link_libraries(${module} ${PYTHON_LIBRARIES} ${ARG_LIBRARIES})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${module} LIBRARY DESTINATION python/lib-dynload)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_EXECUTABLE( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_EXECUTABLE executable)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(exe_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  add_executable( ${executable} ${exe_srcs})
  target_link_libraries(${executable} ${ARG_LIBRARIES} )
  set_target_properties(${executable} PROPERTIES SUFFIX .exe)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${executable} EXPORT ${CMAKE_PROJECT_NAME}Exports RUNTIME DESTINATION ${bin})
  install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake)

endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_UNIT_TEST( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_UNIT_TEST executable)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(exe_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  if(BUILD_TESTS)
    add_executable( ${executable} ${exe_srcs})
    target_link_libraries(${executable} ${ARG_LIBRARIES} )
	SET_RUNTIME_PATH(path ${ld_library_path})
    add_test(${executable} ${env_cmd} -p ${ld_library_path}=${path} ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${executable}.exe )
    #----Installation details-------------------------------------------------------
    set_target_properties(${executable} PROPERTIES SUFFIX .exe)
    install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_FRAMEWORK_TEST( <name> conf1 conf2 ... ENVIRONMENT env=val ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_FRAMEWORK_TEST name)
  if(BUILD_TESTS)
    PARSE_ARGUMENTS(ARG "ENVIRONMENT" "" ${ARGN})
    foreach( optfile  ${ARG_DEFAULT_ARGS} )
      if( IS_ABSOLUTE ${optfile})
        set( optfiles ${optfiles} ${optfile})
      else()
        set( optfiles ${optfiles} ${CMAKE_CURRENT_SOURCE_DIR}/${optfile})
      endif()
    endforeach()
    add_test(${name} ${CMAKE_INSTALL_PREFIX}/scripts/testwrap${ssuffix} ${CMAKE_INSTALL_PREFIX}/setup${ssuffix} "." ${gaudirun} ${optfiles})
    set_property(TEST ${name} PROPERTY ENVIRONMENT
      LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}
      ${ARG_ENVIRONMENT})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_QMTEST_TEST( <name> TESTS qmtest1 qmtest2 ... ENVIRONMENT env=val ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_QMTEST_TEST name)
  if(BUILD_TESTS)
    PARSE_ARGUMENTS(ARG "TESTS;ENVIRONMENT" "" ${ARGN})
    foreach(arg ${ARG_TESTS})
	  set(tests ${tests} ${arg})
    endforeach()
    if( NOT tests )
      set(tests ${name})
    endif()
    GAUDI_USE_PACKAGE(QMtest)
    add_test(${name} ${CMAKE_INSTALL_PREFIX}/scripts/testwrap${ssuffix} ${CMAKE_INSTALL_PREFIX}/setup${ssuffix}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest
                     qmtest run ${tests})
    set_property(TEST ${name} PROPERTY ENVIRONMENT
      LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}
      QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes
      ${ARG_ENVIRONMENT})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_HEADERS([dir1 dir2 ...])
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_HEADERS)
  if( ARGN )
    set( dirs ${ARGN} )
  else()
    get_filename_component(dirs ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  endif()
  foreach( inc ${dirs})
    install(DIRECTORY ${inc}
            DESTINATION include
            FILES_MATCHING
              PATTERN "*.h"
              PATTERN "*.icpp" )
  endforeach()
  set_directory_properties(PROPERTIES INSTALL_HEADERS ON)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_MODULES( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_MODULES)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(DIRECTORY python/
          DESTINATION python
          FILES_MATCHING PATTERN "*.py")
  GAUDI_GENERATE_CONFUSERDB() # if there are Python modules, there may be ConfigurableUser's
  GAUDI_INSTALL_PYTHON_INIT()
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_INIT( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_INIT)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(CODE "if (NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/python/${package}/__init__.py\")
                  file(INSTALL DESTINATION \"${CMAKE_INSTALL_PREFIX}/python/${package}\"
                               TYPE FILE
                               FILES \"${GAUDI_SOURCE_DIR}/GaudiPolicy/cmt/fragments/__init__.py\"  )
                endif()" )
  GAUDI_ZIP_PYTHON_MODULES()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_ZIP_PYTHON_MODULES( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_ZIP_PYTHON_MODULES)
  install(CODE "execute_process(COMMAND  ${zippythondir_cmd} --quiet ${CMAKE_INSTALL_PREFIX}/python)")
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_SCRIPTS( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_SCRIPTS)
  install(DIRECTORY scripts/ DESTINATION scripts
          FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ
          PATTERN ".svn" EXCLUDE
          PATTERN "*~" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_JOBOPTIONS( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_JOBOPTIONS)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(FILES ${ARGN} DESTINATION jobOptions/${package})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_PROJECT_VERSION_HEADER( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_PROJECT_VERSION_HEADER )
  set(project ${CMAKE_PROJECT_NAME})
  set(version ${${CMAKE_PROJECT_NAME}_VERSION})
  set(output  ${BUILD_OUTPUT_PREFIX}/include/${project}_VERSION.h)
  add_custom_command(OUTPUT ${output}
                     COMMAND ${versheader_cmd} ${project} ${version} ${output})
  add_custom_target(${project}VersionHeader ALL
                    DEPENDS ${output})
  install(FILES ${output} DESTINATION include)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_USE_PACKAGE( package )
#---------------------------------------------------------------------------------------------------
macro( GAUDI_USE_PACKAGE package )
  if( EXISTS ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt)
    #include_directories( ${CMAKE_SOURCE_DIR}/${package} )
    file(READ ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt file_contents)
    string( REGEX MATCHALL "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])+" vars ${file_contents})
    foreach( var ${vars})
      string(REGEX REPLACE "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])" "\\1" p ${var})
      #GAUDI_USE_PACKAGE(${p})
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
#---GAUDI_BUILD_SETUP( )
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
#---GAUDI_BUILD_PROJECT_SETUP( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PROJECT_SETUP )
  set( setup  ${CMAKE_BINARY_DIR}/setup${ssuffix} )
  file(WRITE  ${setup} "${scomment} ${CMAKE_PROJECT_NAME} Setup file\n")
  if(WIN32)
	file(APPEND ${setup} "@echo off\n")
    file(APPEND ${setup} "set PATH=${CMAKE_INSTALL_PREFIX}/${bin};${CMAKE_INSTALL_PREFIX}/${lib};${CMAKE_INSTALL_PREFIX}/scripts;%PATH%\n")
    file(APPEND ${setup} "set PYTHONPATH=${CMAKE_INSTALL_PREFIX}/python;%PYTHONPATH%\n")
  else()
    file(APPEND ${setup} "setenv PATH  ${CMAKE_INSTALL_PREFIX}/${bin}:${CMAKE_INSTALL_PREFIX}/scripts:\${PATH}\n")
    file(APPEND ${setup} "setenv LD_LIBRARY_PATH  ${CMAKE_INSTALL_PREFIX}/${lib}:\${LD_LIBRARY_PATH}\n")
    file(APPEND ${setup} "setenv PYTHONPATH  ${CMAKE_INSTALL_PREFIX}/python:\${PYTHONPATH}\n")
  endif()

  #----Get the setup fro each external package
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  get_property(found_projects GLOBAL PROPERTY PROJECTS_FOUND)
  foreach( package ${found_projects} ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${package} "${${package}_environment}")
  endforeach()

  #---Get the setup for each package (directory)
  file(APPEND  ${setup} "\n${scomment} Standard variables for each package\n")
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
  #---Installation---------------------------------------------------------------------------------
  install(FILES ${setup}  DESTINATION .
                          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                      GROUP_EXECUTE GROUP_READ
                                      WORLD_EXECUTE WORLD_READ )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_PACKAGE_SETUP( setupfile package envlist )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PACKAGE_SETUP setup package envlist )
  if ( NOT setup )
    set( setup ${CMAKE_INSTALL_PREFIX}/${package}_setup.csh )
    file(WRITE  ${setup} "${scomment} Package ${package} setup file\n")
  else()
    file(APPEND  ${setup} "\n${scomment} Package ${package} setup file\n")
  endif()
  foreach( env ${envlist} )
    if(env MATCHES ".*[+]=.*")
      string(REGEX REPLACE "([^=+]+)[+]=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*[+]=(.+)" "\\1"  val ${env})
	  if(WIN32)
        file(APPEND ${setup} "set ${var}=${val};%${var}%\n")
	  else()
        file(APPEND ${setup} "if \$?${var} then\n")
        file(APPEND ${setup} "  setenv ${var} ${val}:\${${var}}\n")
        file(APPEND ${setup} "else\n")
        file(APPEND ${setup} "  setenv ${var} ${val}\n")
        file(APPEND ${setup} "endif\n")
	  endif()
    elseif ( env MATCHES ".*=.*")
      string(REGEX REPLACE "([^=+]+)=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*=(.+)" "\\1"  val ${env})
	  if(WIN32)
        file(APPEND ${setup} "set ${var}=${val}\n")
	  else()
        file(APPEND ${setup} "setenv ${var} ${val}\n")
	  endif()
   endif()
  endforeach()
endfunction()
