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
  option(BUILD_DLLEXPORT_LIBS "Set to ON to build 'linker' libraries exporting only declared dll_export symbols" OFF)
  option(BUILD_TESTS "Set to ON to build the tests (libraries and executables)" OFF)
  #-------------------------------------------------------------------------------------------------
  #--- Build type and tag strings-------------------------------------------------------------------

  set(opt2buildtype Release)
  set(dbg2buildtype Debug)
  set(Debug2type dbg)
  set(Release2type opt)

  if(DEFINED ENV{CMAKECONFIG})
    set(tag $ENV{CMAKECONFIG})
  elseif(DEFINED ENV{CMTCONFIG})
    set(tag $ENV{CMTCONFIG})
  else()
    GAUDI_BINARY_TAG(tag)
  endif()
  string(REGEX MATCHALL "[^-]+" out ${tag})
  list(GET out 0 arch)
  list(GET out 1 os)
  list(GET out 2 comp)
  list(GET out 3 type)

  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${${type}2buildtype} CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
  endif()

  set(BINARY_TAG_PREFIX ${arch}-${os}-${comp} CACHE STRING "Installation binary tag prefix. The final tag will be made using the BUILD_TYPE" )
  set(BINARY_TAG ${BINARY_TAG_PREFIX}-${${CMAKE_BUILD_TYPE}2type})

  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/InstallArea/${BINARY_TAG} CACHE PATH
      "Install path prefix, prepended onto install directories." FORCE )
  endif()

  if( NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE STRING
	   "Single build output directory for all executables" FORCE)
  endif()
  if( NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE STRING
	   "Single build output directory for all libraries" FORCE)
  endif()


  if(BUILD_TESTS)
    enable_testing()
  endif()

  #--- Project Installations------------------------------------------------------------------------
  install(DIRECTORY cmake/ DESTINATION cmake
                           FILES_MATCHING PATTERN "*.cmake"
                           PATTERN ".svn" EXCLUDE )
  install(PROGRAMS cmake/testwrap.sh cmake/testwrap.csh cmake/testwrap.bat cmake/genCMake.py cmake/cmdwrap.bat DESTINATION scripts)

  #--- Global actions for the project
  INCLUDE(GaudiPolicy)
  GAUDI_GET_PACKAGES(packages)
  #GAUDI_SORT_PACKAGES(packages ${packages})
  foreach(package ${packages})
    message("-- Adding directory ${package}")
    add_subdirectory(${package})
  endforeach()

  GAUDI_PROJECT_VERSION_HEADER()
  GAUDI_BUILD_PROJECT_SETUP()
  GAUDI_MERGE_CONF_DB()

  #GAUDI_USE_PACKAGE(QMtest)
  #GAUDI_USE_PACKAGE(pytools)
  #GAUDI_USE_PACKAGE(RELAX)
  #SET( QMtest_environment ${QMtest_environment} QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes )
  #GAUDI_PROJECT_VERSION_HEADER()
  #GAUDI_BUILD_PROJECT_SETUP()

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
	  if(WIN32)
        set(dllpath PATH)
      else()
        set(dllpath LD_LIBRARY_PATH)
      endif()
      #------Set the list of variables to make a effective 'use' of the project-----
      get_property(projects GLOBAL PROPERTY PROJECTS_FOUND)
      set_property(GLOBAL PROPERTY PROJECTS_FOUND ${projects} ${project})
      get_property(projects GLOBAL PROPERTY PROJECTS_FOUND)
      set(CMAKE_MODULE_PATH ${${project}_binaryarea}/cmake ${CMAKE_MODULE_PATH})
      include_directories( ${${project}_binaryarea}/include )
      link_directories( ${${project}_binaryarea}/lib )
      set(${project}_environment ${dllpath}+=${${project}_binaryarea}/lib
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
#---GAUDI_MERGE_CONF_DB
#---------------------------------------------------------------------------------------------------
# Take care of the rules to build the merged database of ConfigurableUser
# specializations.
function(GAUDI_MERGE_CONF_DB)
  # Check if one of the packages produces ConfUserDB
  get_property(needed GLOBAL PROPERTY MergedConfDB_SOURCES SET)
  if(needed)
    # get the list of parts to merge
    get_property(parts GLOBAL PROPERTY MergedConfDB_SOURCES)
    # create the targets
    set(output ${CMAKE_BINARY_DIR}/python/${CMAKE_PROJECT_NAME}_merged_confDb.py)
    set(merge_cmd ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/merge_files.py --no-stamp)
    add_custom_command(OUTPUT ${output}
                       COMMAND ${merge_cmd} ${parts} ${output}
                       DEPENDS ${parts})
    add_custom_target(MergedConfDB ALL DEPENDS ${output})
    # prepare the high level dependencies
    get_property(deps GLOBAL PROPERTY MergedConfDB_DEPENDS)
    add_dependencies(MergedConfDB ${deps})
    # prepare the output directory
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/python)
    # install rule for the merged DB
    install(FILES ${output} DESTINATION python)
  endif()
endfunction()
