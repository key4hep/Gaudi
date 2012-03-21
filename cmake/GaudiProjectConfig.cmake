# - GaudiProject
# Define the macros used by Gaudi-based projects, namely:
#  gaudi_project(project version) : declare a project with it's version number
#  gaudi_use_project(project version) : declare the dependency on another project
#
# Authors: Pere Mato, Marco Clemencic

cmake_minimum_required(VERSION 2.8.5)

# Add the directory containing this file to the modules search path
set(CMAKE_MODULE_PATH ${GaudiProject_DIR} ${CMAKE_MODULE_PATH})

#-------------------------------------------------------------------------------
# Basic configuration
#-------------------------------------------------------------------------------
set(CMAKE_VERBOSE_MAKEFILES OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Ensure that the include directories added are always taken first.
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
#set(CMAKE_SKIP_BUILD_RPATH TRUE)

find_program(ccache_cmd ccache)
if(ccache_cmd)
  option(CMAKE_USE_CCACHE "Use ccache to speed up compilation." OFF)
  if(CMAKE_USE_CCACHE)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${ccache_cmd})
    message(STATUS "Using ccache for building")
  endif()
endif()
find_program(distcc_cmd distcc)
if(distcc_cmd)
  option(CMAKE_USE_DISTCC "Use distcc to speed up compilation." OFF)
  if(CMAKE_USE_DISTCC)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${distcc_cmd})
    message(STATUS "Using distcc for building")
    if(CMAKE_USE_CCACHE)
      message(WARNING "Cannot use distcc and ccache at the same time: using distcc")
    endif()
  endif()
endif()
mark_as_advanced(ccache_cmd distcc_cmd)

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
else()
  set(ssuffix .csh)
  set(scomment \#)
endif()


#---------------------------------------------------------------------------------------------------
# Programs and utilities needed for the build
#---------------------------------------------------------------------------------------------------
include(CMakeParseArguments)

find_package(PythonInterp)

#--- commands required to build cached variable
# (python scripts are located as such but run through python)
set(hints ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts ${CMAKE_SOURCE_DIR}/GaudiKernel/scripts ${CMAKE_SOURCE_DIR}/Gaudi/scripts)

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

find_program(gaudirun_cmd gaudirun.py HINTS ${hints})
set(gaudirun_cmd ${PYTHON_EXECUTABLE} ${gaudirun_cmd})

#---------------------------------------------------------------------------------------------------
#---gaudi_project(project version)
#---------------------------------------------------------------------------------------------------
macro(gaudi_project project version)
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
  project(${project})
  #----For some reason this is not set by calling 'project()'
  set(CMAKE_PROJECT_NAME ${project})

  #--- Define the version of the project - can be used to generate sources,
  set(CMAKE_PROJECT_VERSION ${version} CACHE STRING "Version of the project")

  string(REGEX MATCH "v?([0-9]+)[r.]([0-9]+)([p.]([0-9]+))?" _version ${version})
  set(CMAKE_PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1} CACHE INTERNAL "Major version of project")
  set(CMAKE_PROJECT_VERSION_MINOR ${CMAKE_MATCH_2} CACHE INTERNAL "Minor version of project")
  set(CMAKE_PROJECT_VERSION_PATCH ${CMAKE_MATCH_4} CACHE INTERNAL "Patch version of project")

  #--- Project Options and Global settings----------------------------------------------------------
  option(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON)
  option(BUILD_TESTS "Set to ON to build the tests (libraries and executables)" OFF)
  option(HIDE_WARNINGS "Turn on or off options that are used to hide warning messages" ON)
  option(USE_EXE_SUFFIX "Add the .exe suffix to executables on Unix systems (like CMT)" ON)
  #-------------------------------------------------------------------------------------------------

  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/InstallArea/${BINARY_TAG} CACHE PATH
      "Install path prefix, prepended onto install directories." FORCE )
  endif()

  if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING
	   "Single build output directory for all executables" FORCE)
  endif()
  if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE STRING
	   "Single build output directory for all libraries" FORCE)
  endif()

  if(BUILD_TESTS)
    enable_testing()
  endif()

  # FIXME: external tools need to be found independently of the project
  if(CMAKE_PROJECT_NAME STREQUAL Gaudi)
    if (NOT USE_EXE_SUFFIX)
      set(genconf_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genconf)
      set(genwindef_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genwindef)
    else()
      set(genconf_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genconf.exe)
      set(genwindef_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genwindef.exe)
    endif()
  else()
    if (NOT USE_EXE_SUFFIX)
      set(genconf_cmd ${GAUDI_binaryarea}/bin/genconf.exe)
      set(genwindef_cmd ${GAUDI_binaryarea}/bin/genwindef.exe)
    else()
      set(genconf_cmd ${GAUDI_binaryarea}/bin/genconf.exe)
      set(genwindef_cmd ${GAUDI_binaryarea}/bin/genwindef.exe)
    endif()
    set(GAUDI_SOURCE_DIR ${GAUDI_installation})
  endif()

  #--- Project Installations------------------------------------------------------------------------
  install(DIRECTORY cmake/ DESTINATION cmake
                           FILES_MATCHING PATTERN "*.cmake"
                           PATTERN ".svn" EXCLUDE )
  install(PROGRAMS cmake/testwrap.sh cmake/testwrap.csh cmake/testwrap.bat cmake/genCMake.py cmake/env.py DESTINATION scripts)

  #--- Global actions for the project
  include(GaudiContrib)
  include(GaudiBuildFlags)

  message(STATUS "Looking for local directories...")
  gaudi_get_packages(packages)
  message(STATUS "Found:")
  foreach(package ${packages})
    message(STATUS "  ${package}")
  endforeach()

  set(library_path)
  gaudi_sort_subdirectories(packages)
  foreach(package ${packages})
    message(STATUS "Adding directory ${package}")
    add_subdirectory(${package})
  endforeach()

  GAUDI_PROJECT_VERSION_HEADER()
  GAUDI_BUILD_PROJECT_SETUP()
  GAUDI_MERGE_TARGET(ConfDB python ${CMAKE_PROJECT_NAME}_merged_confDb.py)
  GAUDI_MERGE_TARGET(Rootmap lib ${CMAKE_PROJECT_NAME}.rootmap)
  GAUDI_MERGE_TARGET(DictRootmap lib ${CMAKE_PROJECT_NAME}Dict.rootmap)

  gaudi_generate_project_config_version_file()

  gaudi_generate_project_config_file()

  gaudi_generate_project_environment_file()

  gaudi_generate_project_platform_config_file()

  #--- CPack configuration
  set(CPACK_PACKAGE_NAME ${project})
  foreach(t MAJOR MINOR PATCH)
    set(CPACK_PACKAGE_VERSION_${t} ${CMAKE_PROJECT_VERSION_${t}})
  endforeach()
  set(CPACK_SYSTEM_NAME ${BINARY_TAG})

  set(CPACK_GENERATOR TGZ)

  include(CPack)

endmacro()

#-------------------------------------------------------------------------------
# include_package_directories(Package1 [Package2 ...])
#
# Adde the include directories of each package to the include directories.
#-------------------------------------------------------------------------------
function(include_package_directories)
  foreach(package ${ARGN})
    # we need to ensure that the user can call this function also for directories
    if(IS_DIRECTORY ${package})
      include_directories(${package})
    else()
      # ensure that the current directory knows about the package
      find_package(${package} QUIET)
      set(to_incl)
      string(TOUPPER ${package} _pack_upper)
      if(${_pack_upper}_FOUND OR ${package}_FOUND)
        # Handle some special cases first, then try for package uppercase (DIRS and DIR)
        # If the package is found, add INCLUDE_DIRS or (if not defined) INCLUDE_DIR.
        # If none of the two is defined, do not add anything.
        if(${package} STREQUAL PythonLibs)
          set(to_incl PYTHON_INCLUDE_DIRS)
        elseif(${_pack_upper}_INCLUDE_DIRS)
          set(to_incl ${_pack_upper}_INCLUDE_DIRS)
        elseif(${_pack_upper}_INCLUDE_DIR)
          set(to_incl ${_pack_upper}_INCLUDE_DIR)
        elseif(${package}_INCLUDE_DIRS)
          set(to_incl ${package}_INCLUDE_DIRS)
        endif()
        # Include the directories
        include_directories(${${to_incl}})
      endif()
    endif()
  endforeach()
endfunction()

#-------------------------------------------------------------------------------
# gaudi_sort_subdirectories
#-------------------------------------------------------------------------------
# NO-OP function used by gaudi_sort_subdirectories
function(require)
endfunction()
# helper macros to collect the required subdirs
macro(__visit__ _p)
  if(NOT __${_p}_visited__)
    set(__${_p}_visited__ 1)
    #---list all dependent packages-----
    if( EXISTS ${CMAKE_SOURCE_DIR}/${_p}/CMakeLists.txt)
      file(READ ${CMAKE_SOURCE_DIR}/${_p}/CMakeLists.txt file_contents)
      string(REGEX MATCHALL "require *\\(([^)]+)\\)" vars ${file_contents})
      foreach(var ${vars})
        string(REGEX REPLACE "require *\\(([^)]+)\\)" "\\1" __p ${var})
        separate_arguments(__p)
        foreach(___p ${__p})
           __visit__(${___p})
        endforeach()
      endforeach()
      set(out_packages ${out_packages} ${_p})
    endif()
  endif()
endmacro()
# Actual function
function(gaudi_sort_subdirectories var)
  set(out_packages)
  set(in_packages ${${var}})
  foreach(p ${in_packages})
    __visit__(${p})
  endforeach()
  set(${var} ${out_packages} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------
# gaudi_get_packages
#
# Find all the CMakeLists.txt files in the sub-directories and add their
# directories to the variable.
#-------------------------------------------------------------------------------
function(gaudi_get_packages var)
  set(packages)
  file(GLOB_RECURSE cmakelist_files RELATIVE ${CMAKE_SOURCE_DIR} CMakeLists.txt)
  foreach(file ${cmakelist_files})
    # ignore the source directory itself
    if(NOT path STREQUAL CMakeLists.txt)
      get_filename_component(package ${file} PATH)
      SET(packages ${packages} ${package})
    endif()
  endforeach()
  set(${var} ${packages} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------
# gaudi_get_package_name(VAR)
#
# Set the variable VAR to the current "package" (subdirectory) name.
#-------------------------------------------------------------------------------
macro(gaudi_get_package_name VAR)
  # By convention, the package is the name of the source directory.
  get_filename_component(${VAR} ${CMAKE_CURRENT_SOURCE_DIR} NAME)
endmacro()

#-------------------------------------------------------------------------------
# gaudi_resolve_link_libraries(variable lib_or_package1 lib_or_package2 ...)
#
# Translate the package names in a list of link library options into the
# corresponding library options.
# Example:
#
#  find_package(Boost COMPONENTS filesystem regex)
#  find_package(ROOT COMPONENTS RIO)
#  gaudi_resolve_link_libraries(LIBS Boost ROOT)
#  ...
#  target_link_libraries(XYZ ${LIBS})
#
# Note: this function is more useful in wrappers to add_library etc, like
#       gaudi_add_library
#-------------------------------------------------------------------------------
function(gaudi_resolve_link_libraries variable)
  set(collected)
  foreach(package ${ARGN})
    # check if it is an actual library or a target first
    if(TARGET ${package} OR EXISTS ${package})
      set(collected ${collected} ${package})
    else()
      # it must be an available package
      string(TOUPPER ${package} _pack_upper)
      # The case of CMAKE_DL_LIBS is more special than others
      if(${_pack_upper}_FOUND OR ${package}_FOUND)
        # Handle some special cases first, then try for PACKAGE_LIBRARIES
        # otherwise fall back on Package_LIBRARIES.
        if(${package} STREQUAL PythonLibs)
          set(collected ${collected} ${PYTHON_LIBRARIES})
        elseif(${_pack_upper}_LIBRARIES)
          set(collected ${collected} ${${_pack_upper}_LIBRARIES})
        else()
          set(collected ${collected} ${${package}_LIBRARIES})
        endif()
      else()
        # if it's not a package, we just add it as it is... there are a lot of special cases
        set(collected ${collected} ${package})
      endif()
    endif()
  endforeach()
  set(${variable} ${collected} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------
#---GAUDI_MERGE_TARGET
#-------------------------------------------------------------------------------
# Create a MergedXXX target that takes input files and dependencies from
# properties of the packages
function(GAUDI_MERGE_TARGET tgt dest filename)
  # Check if one of the packages produces files for this merge target
  get_property(needed GLOBAL PROPERTY Merged${tgt}_SOURCES SET)
  if(needed)
    # get the list of parts to merge
    get_property(parts GLOBAL PROPERTY Merged${tgt}_SOURCES)
    # create the targets
    set(output ${CMAKE_BINARY_DIR}/${dest}/${filename})
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
# gaudi_generate_configurables(library)
#
# Internal function. Add the targets needed to produce the configurables for a
# module (component library).
#---------------------------------------------------------------------------------------------------
function(gaudi_generate_configurables library)
  gaudi_get_package_name(package)

  # set(library_preload)  # TODO....

  # Prepare the build directory
  set(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})
  file(MAKE_DIRECTORY ${outdir})

  # Python classes used for the various component types.
  set(confModuleName GaudiKernel.Proxy)
  set(confDefaultName Configurable.DefaultName)
  set(confAlgorithm ConfigurableAlgorithm)
  set(confAlgTool ConfigurableAlgTool)
  set(confAuditor ConfigurableAuditor)
  set(confService ConfigurableService)

  add_custom_command(
    OUTPUT ${outdir}/${library}_confDb.py ${outdir}/${library}Conf.py ${outdir}/__init__.py
    COMMAND ${env_cmd}
                  -p ${ld_library_path}=.
                  -p ${ld_library_path}=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
              ${genconf_cmd} ${library_preload} -o ${outdir} -p ${package}
                --configurable-module=${confModuleName}
                --configurable-default-name=${confDefaultName}
                --configurable-algorithm=${confAlgorithm}
                --configurable-algtool=${confAlgTool}
                --configurable-auditor=${confAuditor}
                --configurable-service=${confService}
                -i ${library}
    DEPENDS ${library})
  add_custom_target(${library}Conf ALL DEPENDS ${outdir}/${library}_confDb.py)
  # Add dependencies on GaudiSvc and the genconf executable if they have to be built in the current project
  add_dependencies(${library}Conf genconf GaudiCoreSvc)
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedConfDB_SOURCES ${outdir}/${library}_confDb.py)
  set_property(GLOBAL APPEND PROPERTY MergedConfDB_DEPENDS ${library}Conf)
  #----Installation details-------------------------------------------------------
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConf/
          DESTINATION python
          FILES_MATCHING PATTERN "*.py")
  GAUDI_INSTALL_PYTHON_INIT()
endfunction()

define_property(DIRECTORY
                PROPERTY CONFIGURABLE_USER_MODULES
                BRIEF_DOCS "ConfigurableUser modules"
                FULL_DOCS "List of Python modules containing ConfigurableUser specializations (default <package>/Config, 'None' to disable)." )
#---------------------------------------------------------------------------------------------------
# gaudi_generate_confuserdb([DEPENDS target1 target2])
#
# Generate entries in the configurables database for ConfigurableUser specializations.
# By default, the python module supposed to contain ConfigurableUser's is <package>.Config,
# but different (or more) modules can be specified with the directory property
# CONFIGURABLE_USER_MODULES. If that property is set to None, there will be no
# search for ConfigurableUser's.
#---------------------------------------------------------------------------------------------------
function(gaudi_generate_confuserdb)
  gaudi_get_package_name(package)
  get_directory_property(modules CONFIGURABLE_USER_MODULES)
  if( NOT (modules STREQUAL "None") ) # ConfUser enabled
    set(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})

    # get the optional dependencies from argument and properties
    CMAKE_PARSE_ARGUMENTS(ARG "" "" "DEPENDS" ${arguments})
    get_directory_property(PROPERTY_DEPENDS CONFIGURABLE_USER_DEPENDS)

    # TODO: this re-runs the genconfuser every time
    #       we have to force it because we cannot define the dependencies
    #       correctly (on the Python files)
    add_custom_target(${package}ConfUserDB ALL
                      DEPENDS ${outdir}/${package}_user_confDb.py)
    if(${ARG_DEPENDS} ${PROPERTY_DEPENDS})
      add_dependencies(${package}ConfUserDB ${ARG_DEPENDS} ${PROPERTY_DEPENDS})
    endif()
    add_custom_command(
      OUTPUT ${outdir}/${package}_user_confDb.py
      COMMAND ${env_cmd}
                    -p PYTHONPATH=${CMAKE_SOURCE_DIR}/GaudiKernel/python
                    -p PYTHONPATH=${CMAKE_SOURCE_DIR}/Gaudi/python
                ${genconfuser_cmd}
                  -r ${CMAKE_CURRENT_SOURCE_DIR}/python
                  -o ${outdir}/${package}_user_confDb.py
                  ${package} ${modules})
    set_property(GLOBAL APPEND PROPERTY MergedConfDB_SOURCES ${outdir}/${package}_user_confDb.py)
    set_property(GLOBAL APPEND PROPERTY MergedConfDB_DEPENDS ${package}ConfUserDB)
  endif()
endfunction()

#-------------------------------------------------------------------------------
# gaudi_get_required_include_dirs(<output> <libraries>)
#
# Get the include directories required by the linker libraries specified
# and prepend them to the output variable.
#-------------------------------------------------------------------------------
function(gaudi_get_required_include_dirs output)
  set(collected)
  foreach(lib ${ARGN})
    set(req)
    if(TARGET ${lib})
      get_property(req TARGET ${lib} PROPERTY REQUIRED_INCLUDE_DIRS)
      if(req)
        list(APPEND collected ${req})
      endif()
    endif()
  endforeach()
  if(collected)
    set(${output} ${collected} ${${output}} PARENT_SCOPE)
  endif()
endfunction()

#-------------------------------------------------------------------------------
# gaudi_get_required_library_dirs(<output> <libraries>)
#
# Get the library directories required by the linker libraries specified
# and prepend them to the output variable.
#-------------------------------------------------------------------------------
function(gaudi_get_required_library_dirs output)
  set(collected)
  foreach(lib ${ARGN})
    set(req)
    if(EXISTS ${lib})
      get_filename_component(req ${lib} PATH)
      if(req)
        list(APPEND collected ${req})
      endif()
      # FIXME: we should handle the inherited targets
      # (but it's not mandatory because they where already handled)
    endif()
  endforeach()
  if(collected)
    set(${output} ${collected} ${${output}} PARENT_SCOPE)
  endif()
endfunction()

#-------------------------------------------------------------------------------
# gaudi_expand_sources(<variable> source_pattern1 source_pattern2 ...)
#
# Get the library directories required by the linker libraries specified
# and prepend them to the output variable.
#-------------------------------------------------------------------------------
macro(gaudi_expand_sources VAR)
  #message(STATUS "Expand ${ARGN} in ${VAR}")
  set(${VAR})
  foreach(fp ${ARGN})
    file(GLOB files src/${fp})
    if(files)
      set(${VAR} ${${VAR}} ${files})
    else()
      set(${VAR} ${${VAR}} ${fp})
    endif()
  endforeach()
  #message(STATUS "  result: ${${VAR}}")
endmacro()

#-------------------------------------------------------------------------------
# gaudi_common_add_build(sources...
#                 LINK_LIBRARIES library1 package2 ...
#                 INCLUDE_DIRS dir1 package2 ...)
#
# Internal. Helper macro to factor out the common code to configure a buildable
# target (library, module, dictionary...)
#-------------------------------------------------------------------------------
macro(gaudi_common_add_build)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LIBRARIES;LINK_LIBRARIES;INCLUDE_DIRS" ${ARGN})
  # obsolete option
  if(ARG_LIBRARIES)
    message(WARNING "Deprecated option 'LIBRARY', use 'LINK_LIBRARIES' instead")
    set(ARG_LINK_LIBRARIES ${ARG_LINK_LIBRARIES} ${ARG_LIBRARIES})
  endif()

  gaudi_resolve_link_libraries(ARG_LINK_LIBRARIES ${ARG_LINK_LIBRARIES})

  # find the sources
  gaudi_expand_sources(srcs ${ARG_UNPARSED_ARGUMENTS})

  # get the inherited include directories
  gaudi_get_required_include_dirs(ARG_INCLUDE_DIRS ${ARG_LINK_LIBRARIES})

  # add the package includes to the current list
  include_package_directories(${ARG_INCLUDE_DIRS})

  # get the library dirs required to get the libraries we use
  gaudi_get_required_library_dirs(lib_path ${ARG_LINK_LIBRARIES})
  set_property(GLOBAL APPEND PROPERTY LIBRARY_PATH ${lib_path})
endmacro()

#---------------------------------------------------------------------------------------------------
# gaudi_add_library(<name>
#                   source1 source2 ...
#                   LINK_LIBRARIES library1 library2 ...
#                   INCLUDE_DIRS dir1 package2 ...
#                   [NO_PUBLIC_HEADERS | PUBLIC_HEADERS dir1 dir2 ...])
#
# Extension of standard CMake 'add_library' command.
# Create a library from the specified sources (glob patterns are allowed), linking
# it with the libraries specified and adding the include directories to the search path.
#---------------------------------------------------------------------------------------------------
function(gaudi_add_library library)
  # this function uses an extra option: 'PUBLIC_HEADERS'
  CMAKE_PARSE_ARGUMENTS(ARG "NO_PUBLIC_HEADERS" "" "LIBRARIES;LINK_LIBRARIES;INCLUDE_DIRS;PUBLIC_HEADERS" ${ARGN})
  gaudi_common_add_build(${ARG_UNPARSED_ARGUMENTS} LIBRARIES ${ARG_LIBRARIES} LINK_LIBRARIES ${ARG_LINK_LIBRARIES} INCLUDE_DIRS ${ARG_INCLUDE_DIRS})

  if(NOT ARG_NO_PUBLIC_HEADERS AND NOT ARG_PUBLIC_HEADERS)
    gaudi_get_package_name(package)
    message(WARNING "Library ${library} (in ${package}) does not declare PUBLIC_HEADERS")
  endif()

  if(WIN32)
	add_library( ${library}-arc STATIC EXCLUDE_FROM_ALL ${srcs})
    set_target_properties(${library}-arc PROPERTIES COMPILE_DEFINITIONS GAUDI_LINKER_LIBRARY)
    add_custom_command(
      OUTPUT ${library}.def
	  COMMAND ${genwindef_cmd} -o ${library}.def -l ${library} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${library}-arc.lib
	  DEPENDS ${library}-arc genwindef)
	#---Needed to create a dummy source file to please Windows IDE builds with the manifest
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/${library}.cpp "// empty file\n" )
    add_library( ${library} SHARED ${library}.cpp ${library}.def)
    target_link_libraries(${library} ${library}-arc ${ARG_LINK_LIBRARIES})
    set_target_properties(${library} PROPERTIES LINK_INTERFACE_LIBRARIES "${ARG_LINK_LIBRARIES}" )
  else()
    add_library(${library} ${srcs})
    set_target_properties(${library} PROPERTIES COMPILE_DEFINITIONS GAUDI_LINKER_LIBRARY)
    target_link_libraries(${library} ${ARG_LINK_LIBRARIES})
  endif()

  # Declare that the used headers are needed by the libraries linked against this one
  set_property(TARGET ${library} PROPERTY REQUIRED_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_INCLUDE_DIRS})
  set_property(GLOBAL APPEND PROPERTY LINKER_LIBRARIES ${library})

  if(TARGET ${library}Obj2doth)
    add_dependencies( ${library} ${library}Obj2doth)
  endif()
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION  ${lib})
  gaudi_install_headers(${ARG_PUBLIC_HEADERS})
  install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake)
endfunction()

# Backward compatibility macro
macro(gaudi_linker_library)
  message(WARNING "Deprecated function 'gaudi_linker_library', use 'gaudi_add_library' instead")
  gaudi_add_library(${ARGN})
endmacro()

#---------------------------------------------------------------------------------------------------
#---gaudi_add_module(<name> source1 source2 ... LINK_LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(gaudi_add_module library)
  gaudi_common_add_build(${ARGN})

  add_library(${library} MODULE ${srcs})
  target_link_libraries(${library} ${ROOT_Reflex_LIBRARY} ${ARG_LINK_LIBRARIES})

  GAUDI_GENERATE_ROOTMAP(${library})
  gaudi_generate_configurables(${library})

  set_property(GLOBAL APPEND PROPERTY COMPONENT_LIBRARIES ${library})

  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} LIBRARY DESTINATION ${lib})
endfunction()

# Backward compatibility macro
macro(gaudi_component_library)
  message(WARNING "Deprecated function 'gaudi_component_library', use 'gaudi_add_module' instead")
  gaudi_add_module(${ARGN})
endmacro()

#-------------------------------------------------------------------------------
# gaudi_add_dictionary(dictionary header selection
#                      LINK_LIBRARIES ...
#                      INCLUDE_DIRS ...
#                      OPTIONS ...)
#
# Find all the CMakeLists.txt files in the sub-directories and add their
# directories to the variable.
#-------------------------------------------------------------------------------
function(gaudi_add_dictionary dictionary header selection)
  # this function uses an extra option: 'OPTIONS'
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LIBRARIES;LINK_LIBRARIES;INCLUDE_DIRS;OPTIONS" ${ARGN})
  gaudi_common_add_build(${ARG_UNPARSED_ARGUMENTS} LIBRARIES ${ARG_LIBRARIES} LINK_LIBRARIES ${ARG_LINK_LIBRARIES} INCLUDE_DIRS ${ARG_INCLUDE_DIRS})

  reflex_dictionary(${dictionary} ${header} ${selection} LINK_LIBRARIES ${ARG_LINK_LIBRARIES} OPTIONS ${ARG_OPTIONS})

  # Notify the project level target
  get_property(rootmapname TARGET ${dictionary}Gen PROPERTY ROOTMAPFILE)
  set_property(GLOBAL APPEND PROPERTY MergedDictRootmap_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname})
  set_property(GLOBAL APPEND PROPERTY MergedDictRootmap_DEPENDS ${dictionary}Gen)

  #----Installation details-------------------------------------------------------
  install(TARGETS ${dictionary}Dict LIBRARY DESTINATION ${lib})
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_add_python_module(name
#                         sources ...
#                         LINK_LIBRARIES ...
#                         INCLUDE_DIRS ...)
#
# Build a binary python module from the given sources.
#---------------------------------------------------------------------------------------------------
function(gaudi_add_python_module module)
  gaudi_common_add_build(${ARGN})

  # require Python libraries
  find_package(PythonLibs QUIET REQUIRED)

  add_library(${module} MODULE ${srcs})
  if(win32)
    set_target_properties(${module} PROPERTIES SUFFIX .pyd PREFIX "")
  else()
    set_target_properties(${module} PROPERTIES SUFFIX .so PREFIX "")
  endif()
  target_link_libraries(${module} ${PYTHON_LIBRARIES} ${ARG_LINK_LIBRARIES})

  #----Installation details-------------------------------------------------------
  install(TARGETS ${module} LIBRARY DESTINATION python/lib-dynload)
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_add_executable(<name>
#                      source1 source2 ...
#                      LINK_LIBRARIES library1 library2 ...
#                      INCLUDE_DIRS dir1 package2 ...)
#
# Extension of standard CMake 'add_executable' command.
# Create a library from the specified sources (glob patterns are allowed), linking
# it with the libraries specified and adding the include directories to the search path.
#---------------------------------------------------------------------------------------------------
function(gaudi_add_executable executable)
  gaudi_common_add_build(${ARGN})

  add_executable(${executable} ${srcs})
  target_link_libraries(${executable} ${ARG_LINK_LIBRARIES})

  if (USE_EXE_SUFFIX)
    set_target_properties(${executable} PROPERTIES SUFFIX .exe)
  endif()

  #----Installation details-------------------------------------------------------
  install(TARGETS ${executable} EXPORT ${CMAKE_PROJECT_NAME}Exports RUNTIME DESTINATION ${bin})
  install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake)

endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_add_unit_test(<name>
#                     source1 source2 ...
#                     LINK_LIBRARIES library1 library2 ...
#                     INCLUDE_DIRS dir1 package2 ...)
#
# Special version of gaudi_add_executable which automatically adds the dependency
# on CppUnit.
#---------------------------------------------------------------------------------------------------
function(gaudi_add_unit_test executable)
  if(BUILD_TESTS)
    gaudi_common_add_build(${ARGN})

    find_package(CppUnit QUIET REQUIRED)

    gaudi_add_executable(${executable} ${srcs}
                         LINK_LIBRARIES ${ARG_LINK_LIBRARIES} CppUnit
                         INCLUDE_DIRS ${ARG_INCLUDE_DIRS} CppUnit)

    gaudi_get_package_name(package)

    get_target_property(exec_suffix ${executable} SUFFIX)
    if(NOT exec_suffix)
      set(exec_suffix)
    endif()
    add_test(${package}.${executable}
             ${env_cmd}
                 -p ${ld_library_path}=.
                 -p ${ld_library_path}=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                 -p PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
               ${executable}${exec_suffix})
    #----Installation details-------------------------------------------------------
    install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
  endif()
endfunction()

#-------------------------------------------------------------------------------
# gaudi_add_test(<name>
#                [FRAMEWORK options1 options2 ...|QMTEST|COMMAND cmd args ...]
#                [ENVIRONMENT variable=value ...])
#
#-------------------------------------------------------------------------------
function(gaudi_add_test name)
  CMAKE_PARSE_ARGUMENTS(ARG "QMTEST" "" "ENVIRONMENT;FRAMEWORK;COMMAND" ${ARGN})

  gaudi_get_package_name(package)

  if(ARG_QMTEST)
    set(ARG_ENVIRONMENT ${ARG_ENVIRONMENT}
                        QMTESTLOCALDIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest
                        QMTESTRESULTS=${CMAKE_CURRENT_BINARY_DIR}/tests/qmtest/results.qmr
                        QMTESTRESULTSDIR=${CMAKE_CURRENT_BINARY_DIR}/tests/qmtest)
    set(cmdline run_qmtest.py ${package})

  elseif(ARG_FRAMEWORK)
    foreach(optfile  ${ARG_FRAMEWORK})
      if(IS_ABSOLUTE ${optfile})
        set(optfiles ${optfiles} ${optfile})
      else()
        set(optfiles ${optfiles} ${CMAKE_CURRENT_SOURCE_DIR}/${optfile})
      endif()
    endforeach()
    set(cmdline ${gaudirun_cmd} ${optfiles})

  elseif(ARG_COMMAND)
    set(cmdline ${ARG_COMMAND})

  else()
    message(FATAL_ERROR "Type of test '${name}' not declared")
  endif()

  foreach(var ${ARG_ENVIRONMENT})
    set(extra_env ${extra_env} -s ${var})
  endforeach()

  # FIXME: the runtime environment is hacked
  find_package(RELAX QUIET)
  # Boost_LIBRARY_DIR contains 2 entries: optimized and debug
  list(GET Boost_LIBRARY_DIRS 0 bld)
  list(GET RELAX_LIBRARY_DIRS 0 rld)

  add_test(${package}.${name}
           ${env_cmd}
               ${extra_env}
               -p ${ld_library_path}=.
               -p ${ld_library_path}=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
               -p PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

               -p QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes
               -s GAUDI_QMTEST_HTML_OUTPUT=${CMAKE_BINARY_DIR}/test_results

               -p PATH=${CMAKE_INSTALL_PREFIX}/scripts
               -p PYTHONPATH=${CMAKE_INSTALL_PREFIX}/python
               -p PYTHONPATH=${CMAKE_INSTALL_PREFIX}/python/lib-dynload

               -p ${ld_library_path}=${ROOTSYS}/lib
               -p ${ld_library_path}=${rld}
               -p ${ld_library_path}=${bld}
               -p PYTHONPATH=${ROOTSYS}/lib
               ${cmdline})
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_install_headers(dir1 dir2 ...)
#
# Install the declared directories in the 'include' directory.
# To be used in case the header files do not have a library.
#---------------------------------------------------------------------------------------------------
function(gaudi_install_headers)
  foreach(hdr_dir ${ARGN})
    install(DIRECTORY ${hdr_dir}
            DESTINATION include
            FILES_MATCHING
              PATTERN "*.h"
              PATTERN "*.icpp")
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_MODULES( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_MODULES)
  gaudi_get_package_name(package)
  install(DIRECTORY python/
          DESTINATION python
          FILES_MATCHING PATTERN "*.py")
  gaudi_generate_confuserdb() # if there are Python modules, there may be ConfigurableUser's
  GAUDI_INSTALL_PYTHON_INIT()
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_INIT( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_INIT)
  gaudi_get_package_name(package)
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
  gaudi_get_package_name(package)
  install(FILES ${ARGN} DESTINATION jobOptions/${package})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_PROJECT_VERSION_HEADER( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_PROJECT_VERSION_HEADER )
  string(TOUPPER ${CMAKE_PROJECT_NAME} project)
  set(version ${CMAKE_PROJECT_VERSION})
  set(output  ${CMAKE_BINARY_DIR}/include/${project}_VERSION.h)
  add_custom_command(OUTPUT ${output}
                     COMMAND ${versheader_cmd} ${project} ${version} ${output})
  add_custom_target(${project}VersionHeader ALL
                    DEPENDS ${output})
  install(FILES ${output} DESTINATION include)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_PACKAGE_VERSION_HEADER( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_PACKAGE_VERSION_HEADER package version outdir)
  set(output  ${outdir}/${package}Version.h)
  add_custom_command(OUTPUT ${output}
                     COMMAND ${versheader_cmd} ${package} ${version} ${output})
  add_custom_target(${package}VersionHeader ALL
                    DEPENDS ${output})
  #install(FILES ${output} DESTINATION include)
endfunction()

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

#-------------------------------------------------------------------------------
# gaudi_generate_project_config_version_file()
#
# Create the file used by CMake to check if the found version of a package
# matches the requested one.
#-------------------------------------------------------------------------------
macro(gaudi_generate_project_config_version_file)
  message(STATUS "Generating ${CMAKE_PROJECT_NAME}ConfigVersion.cmake")

  if(CMAKE_PROJECT_VERSION_PATCH)
    set(vers_id ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})
  else()
    set(vers_id ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR})
  endif()

  file(WRITE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
"set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(PACKAGE_VERSION ${vers_id})
if((PACKAGE_NAME STREQUAL PACKAGE_FIND_NAME)
   AND (PACKAGE_VERSION STREQUAL PACKAGE_FIND_VERSION))
  set(PACKAGE_VERSION_EXACT 1)
  set(PACKAGE_VERSION_COMPATIBLE 1)
  set(PACKAGE_VERSION_UNSUITABLE 0)
else()
  set(PACKAGE_VERSION_EXACT 0)
  set(PACKAGE_VERSION_COMPATIBLE 0)
  set(PACKAGE_VERSION_UNSUITABLE 1)
endif()
")
  install(FILES ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake DESTINATION ${CMAKE_SOURCE_DIR})
endmacro()

#-------------------------------------------------------------------------------
# gaudi_generate_project_config_file()
#
# Generate the config file used by the other projects using this one.
#-------------------------------------------------------------------------------
macro(gaudi_generate_project_config_file)
  message(STATUS "Generating ${CMAKE_PROJECT_NAME}Config.cmake")
  file(WRITE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
"# File automatically generated: DO NOT EDIT.
set(LCG_version ${LCG_version})

if(IS_DIRECTORY \${${CMAKE_PROJECT_NAME}_DIR}/InstallArea/\${LCG_platform}/cmake)
  list(INSERT CMAKE_MODULE_PATH 0 \${${CMAKE_PROJECT_NAME}_DIR}/InstallArea/\${LCG_platform}/cmake)
else()
  message(FATAL_ERROR \"Cannot find \${${CMAKE_PROJECT_NAME}_DIR}/InstallArea/\${LCG_platform}/cmake: platform not supported\")
endif()

set(${CMAKE_PROJECT_NAME}_VERSION ${CMAKE_PROJECT_VERSION})
set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set(${CMAKE_PROJECT_NAME}_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
set(${CMAKE_PROJECT_NAME}_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})

include(${CMAKE_PROJECT_NAME}PlatformConfig)
include(${CMAKE_PROJECT_NAME}Environment)
")
  install(FILES ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake DESTINATION ${CMAKE_SOURCE_DIR})
endmacro()

#-------------------------------------------------------------------------------
# gaudi_generate_project_platform_config_file()
#
# Generate the platform(build)-specific config file included by the other
# projects using this one.
#-------------------------------------------------------------------------------
macro(gaudi_generate_project_platform_config_file)
  message(STATUS "Generating ${CMAKE_PROJECT_NAME}PlatformConfig.cmake")

  # collecting infos
  get_property(linker_libraries GLOBAL PROPERTY LINKER_LIBRARIES)
  get_property(component_libraries GLOBAL PROPERTY COMPONENT_LIBRARIES)

  string(TOUPPER ${CMAKE_PROJECT_NAME} _proj_upper)

  set(filename ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}PlatformConfig.cmake)
  file(WRITE ${filename}
"# File automatically generated: DO NOT EDIT.

# Get the exported informations about the targets
get_filename_component(_dir "\${CMAKE_CURRENT_LIST_FILE}" PATH)
include(\${_dir}/${CMAKE_PROJECT_NAME}Exports.cmake)

# Set useful properties
get_filename_component(_dir "\${_dir}" PATH)
set(${_proj_upper}_INCLUDE_DIRS \${_dir}/include)
set(${_proj_upper}_LIBRARY_DIRS \${_dir}/lib)

set(${_proj_upper}_BINARY_PATH \${_dir}/bin)
if(EXISTS \${_dir}/python.zip)
  set(${_proj_upper}_PYTHON_PATH \${_dir}/python.zip)
else()
  set(${_proj_upper}_PYTHON_PATH \${_dir}/python)
endif()

set(${_proj_upper}_COMPONENT_LIBRARIES ${component_libraries})
set(${_proj_upper}_LINKER_LIBRARIES ${linker_libraries})

# Add special properties to the targets
")

  foreach(library ${linker_libraries})
    get_property(use_headers TARGET ${library} PROPERTY REQUIRED_INCLUDE_DIRS)
    file(APPEND ${filename}
      "set_property(TARGET ${library} PROPERTY REQUIRED_INCLUDE_DIRS ${use_headers})\n")
  endforeach()

  file(APPEND ${filename} "\n# These are not really needed because we import the targets\n")
  foreach(library ${linker_libraries})
    get_property(use_headers TARGET ${library} PROPERTY REQUIRED_INCLUDE_DIRS)
    file(APPEND ${filename}
      "find_library(${_proj_upper}_${library}_LIBRARY ${library} PATHS \${${_proj_upper}_LIBRARY_DIR} NO_DEFAULT_PATH)\n")
  endforeach()

  install(FILES ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}PlatformConfig.cmake DESTINATION cmake)
endmacro()

#-------------------------------------------------------------------------------
# gaudi_generate_project_environment_file()
#
# Generate the config file used by the other projects using this one.
#-------------------------------------------------------------------------------
macro(gaudi_generate_project_environment_file)
  message(STATUS "Generating ${CMAKE_PROJECT_NAME}Environment.cmake")

  # collecting environment infos
  set(python_path)
  set(binary_path)
  set(environment)
  set(library_path2)

  get_property(packages_found GLOBAL PROPERTY PACKAGES_FOUND)
  message("${packages_found}")
  foreach(pack ${packages_found} ${packages})
    # this is needed to get the non-cache variables for the packages
    if(NOT pack STREQUAL GaudiProject)
      find_package(${pack} QUIET)
    endif()
    string(TOUPPER ${pack} _pack_upper)
    if(${_pack_upper}_PYTHON_PATH)
      list(APPEND python_path ${${_pack_upper}_PYTHON_PATH})
    endif()
    if(${_pack_upper}_BINARY_PATH)
      list(APPEND binary_path ${${_pack_upper}_BINARY_PATH})
    endif()
    if(${_pack_upper}_ENVIRONMENT)
      list(APPEND environment ${${_pack_upper}_ENVIRONMENT})
    endif()
    if(${_pack_upper}_LIBRARY_DIRS)
      list(APPEND library_path2 ${${_pack_upper}_LIBRARY_DIRS})
    endif()
    # FIXME: this is too special
    if(pack STREQUAL PythonInterp)
      get_filename_component(bin_path ${PYTHON_EXECUTABLE} PATH)
      list(APPEND binary_path ${bin_path})
    endif()
  endforeach()

  get_property(library_path GLOBAL PROPERTY LIBRARY_PATH)
  foreach(var library_path python_path binary_path environment library_path2)
    if(${var})
      list(REMOVE_DUPLICATES ${var})
    endif()
  endforeach()

  message("BINARY_PATH: ${binary_path}")
  message("PYTHON_PATH: ${python_path}")
  message("LIBRARY_PATH: ${library_path}")
  message("LIBRARY_PATH2: ${library_path2}")
  message("ENVIRONMENT: ${environment}")

  string(TOUPPER ${CMAKE_PROJECT_NAME} _proj_upper)

  file(WRITE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Environment.cmake
"set(${_proj_upper}_BINARY_PATH ${binary_path} CACHE INTERNAL \"\")
set(${_proj_upper}_PYTHON_PATH ${python_path} CACHE INTERNAL \"\")
set(${_proj_upper}_LIBRARY_PATH ${library_path} CACHE INTERNAL \"\")
set(${_proj_upper}_ENVIRONMENT ${environment} CACHE INTERNAL \"\")
")
  install(FILES ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Environment.cmake DESTINATION cmake)
endmacro()
