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

#-------------------------------------------------------------------------------
# gaudi_project(project version)
#
# Main macro for a Gaudi-based project.
# Each project must call this macro once in the top-level CMakeLists.txt,
# stating the project name and the version in the LHCb format (vXrY[pZ]).
#-------------------------------------------------------------------------------
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

  set(env_xml ${CMAKE_BINARY_DIR}/${project}Environment.xml
      CACHE STRING "path to the XML file for the environment")
  mark_as_advanced(env_xml)

  if(BUILD_TESTS)
    enable_testing()
  endif()

  #--- Find subdirectories
  message(STATUS "Looking for local directories...")
  # Locate packages
  gaudi_get_packages(packages)
  message(STATUS "Found:")
  foreach(package ${packages})
    message(STATUS "  ${package}")
  endforeach()

  # List of all known packages, including those exported by other projects
  set(known_packages ${packages})
  #message(STATUS "known_packages (initial) ${known_packages}")

  # paths where to locate scripts and executables
  # Note: it's a bit a duplicate of the one used in gaudi_external_project_environment
  #       but we need it here because the other one is meant to include also
  #       the external libraries requyired by the subdirectories.
  set(binary_paths)

  # Note: the indirection is needed because we are in a macro and not in a function
  #       but the variable 'PROJECT_USES' is propagated in the exports.
  set(PROJECT_USES ${ARGN})
  # we proceed only if we do have extra arguments
  if(PROJECT_USES)
    list(GET PROJECT_USES 0 el)
    if(NOT el STREQUAL "USE")
      message(FATAL_ERROR "Wrong argument ${el}: expected 'USE'")
    else()
      list(REMOVE_AT PROJECT_USES 0)
      _gaudi_use_other_projects(${PROJECT_USES})
    endif()
  endif()

  #--- commands required to build cached variable
  # (python scripts are located as such but run through python)
  set(binary_paths ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts ${CMAKE_SOURCE_DIR}/GaudiKernel/scripts ${CMAKE_SOURCE_DIR}/Gaudi/scripts ${binary_paths})

  find_program(env_cmd env.py PATHS ${binary_paths})
  set(env_cmd ${PYTHON_EXECUTABLE} ${env_cmd})

  find_program(merge_cmd merge_files.py PATHS ${binary_paths})
  set(merge_cmd ${PYTHON_EXECUTABLE} ${merge_cmd} --no-stamp)

  find_program(versheader_cmd createProjVersHeader.py PATHS ${binary_paths})
  set(versheader_cmd ${PYTHON_EXECUTABLE} ${versheader_cmd})

  find_program(genconfuser_cmd genconfuser.py PATHS ${binary_paths})
  set(genconfuser_cmd ${PYTHON_EXECUTABLE} ${genconfuser_cmd})

  find_program(zippythondir_cmd ZipPythonDir.py PATHS ${binary_paths})
  set(zippythondir_cmd ${PYTHON_EXECUTABLE} ${zippythondir_cmd})

  find_program(gaudirun_cmd gaudirun.py PATHS ${binary_paths})
  set(gaudirun_cmd ${PYTHON_EXECUTABLE} ${gaudirun_cmd})

  # genconf is special because it must be known before we actually declare the
  # target in GaudiKernel/src/Util (because we need to be dynamic and agnostic).
  if(TARGET genconf)
    get_target_property(genconf_cmd genconf IMPORTED_LOCATION)
  else()
    if (NOT USE_EXE_SUFFIX)
      set(genconf_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genconf)
    else()
      set(genconf_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genconf.exe)
    endif()
  endif()
  # same as genconf (but it might never be built because it's needed only on WIN32)
  if(TARGET genwindef)
    get_target_property(genwindef_cmd genwindef IMPORTED_LOCATION)
  else()
    if (NOT USE_EXE_SUFFIX)
      set(genwindef_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genwindef)
    else()
      set(genwindef_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genwindef.exe)
    endif()
  endif()

  #--- Project Installations------------------------------------------------------------------------
  install(DIRECTORY cmake/ DESTINATION cmake
                           FILES_MATCHING PATTERN "*.cmake"
                           PATTERN ".svn" EXCLUDE )
  install(PROGRAMS cmake/testwrap.sh cmake/testwrap.csh cmake/testwrap.bat cmake/genCMake.py cmake/env.py DESTINATION scripts OPTIONAL)

  #--- Global actions for the project
  include(GaudiBuildFlags)
  gaudi_project_version_header()

  #--- Collect settings for subdirectories
  set(library_path)
  # Take into account the dependencies between local subdirectories before
  # adding them to the build.
  gaudi_sort_subdirectories(packages)
  # Add all subdirectories to the project build.
  foreach(package ${packages})
    message(STATUS "Adding directory ${package}")
    add_subdirectory(${package})
  endforeach()

  #--- Special global targets for merging files.
  gaudi_merge_files(ConfDB python ${CMAKE_PROJECT_NAME}_merged_confDb.py)
  gaudi_merge_files(Rootmap lib ${CMAKE_PROJECT_NAME}.rootmap)
  gaudi_merge_files(DictRootmap lib ${CMAKE_PROJECT_NAME}Dict.rootmap)

  # FIXME: it is not possible to produce the file python.zip at installation time
  # because of http://public.kitware.com/Bug/view.php?id=8438
  # install(CODE "execute_process(COMMAND  ${zippythondir_cmd} ${CMAKE_INSTALL_PREFIX}/python)")
  add_custom_target(python.zip
                    COMMAND ${zippythondir_cmd} ${CMAKE_INSTALL_PREFIX}/python
                    COMMENT "Zipping Python modules")

  #--- Prepare environment configuration
  message(STATUS "Preparing environment configuration:")
  set(project_environment)

  # - collect environment from externals
  gaudi_external_project_environment()

  message(STATUS "  environment for local subdirectories")
  # - collect internal environment
  #   - project root (for relocatability)
  string(TOUPPER ${project} _proj)
  set(project_environment ${project_environment} SET ${_proj}_PROJECT_ROOT ${CMAKE_SOURCE_DIR})
  #   - 'packages' roots (backward compatibility)
  foreach(package ${packages})
    message(STATUS "    ${package}")
    get_filename_component(_pack ${package} NAME)
    string(TOUPPER ${_pack} _pack)
    set(project_environment ${project_environment} SET ${_pack}ROOT \${${_proj}_PROJECT_ROOT}/${package})
  endforeach()
  #   - 'packages' environments
  foreach(package ${packages})
    get_property(_pack_env DIRECTORY ${package} PROPERTY ENVIRONMENT)
    set(project_environment ${project_environment} ${_pack_env})
  endforeach()

  message(STATUS "  environment for the project")
  #   - installation dirs
  set(project_environment ${project_environment}
        PREPEND PATH \${${_proj}_PROJECT_ROOT}/InstallArea/${BINARY_TAG}/scripts
        PREPEND PATH \${${_proj}_PROJECT_ROOT}/InstallArea/${BINARY_TAG}/bin
        PREPEND LD_LIBRARY_PATH \${${_proj}_PROJECT_ROOT}/InstallArea/${BINARY_TAG}/lib
        PREPEND PYTHONPATH \${${_proj}_PROJECT_ROOT}/InstallArea/${BINARY_TAG}/python
        PREPEND PYTHONPATH \${${_proj}_PROJECT_ROOT}/InstallArea/${BINARY_TAG}/python/lib-dynload)

  # - produce environment XML description
  gaudi_generate_env_conf(${env_xml} ${project_environment})
  install(FILES ${env_xml} DESTINATION .)

  #--- Generate config files to be imported by other projects.
  gaudi_generate_project_config_version_file()
  gaudi_generate_project_config_file()
  gaudi_generate_project_platform_config_file()
  gaudi_generate_exports(${packages})

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
# _gaudi_use_other_projects([project version [project version]...])
#
# Internal macro implementing the handline of the "USE" option.
# (improve readability)
#-------------------------------------------------------------------------------
macro(_gaudi_use_other_projects)
  message(STATUS "Looking for projects:")

  # this is neede because of the way variable expansion works in macros
  set(ARGN_ ${ARGN})
  while(ARGN_)
    list(LENGTH ARGN_ len)
    if(len LESS 2)
      message(FATAL_ERROR "Wrong number of arguments to USE option")
    endif()
    list(GET ARGN_ 0 other_project)
    list(GET ARGN_ 1 other_project_version)
    list(REMOVE_AT ARGN_ 0 1)

    string(REGEX MATCH "v?([0-9]+)[r.]([0-9]+)([p.]([0-9]+))?" _version ${other_project_version})

    set(other_project_cmake_version ${CMAKE_MATCH_1}.${CMAKE_MATCH_2})
    if(NOT CMAKE_MATCH_4 STREQUAL "")
      set(other_project_cmake_version ${other_project_cmake_version}.${CMAKE_MATCH_4})
    endif()

    if(NOT ${other_project}_FOUND)
      find_package(${other_project} ${other_project_cmake_version} HINTS ..)
      if(${other_project}_FOUND)
        message(STATUS "  found ${other_project} ${${other_project}_VERSION} ${${other_project}_DIR}")
        if(NOT heptools_version STREQUAL ${other_project}_heptools_version)
          if(${other_project}_heptools_version)
            set(hint_message "with the option '-DCMAKE_TOOLCHAIN_FILE=.../heptools-${${other_project}_heptools_version}.cmake'")
          else()
            set(hint_message "without the option '-DCMAKE_TOOLCHAIN_FILE=...'")
          endif()
          message(FATAL_ERROR "Incompatible versions of heptools toolchains:
  ${CMAKE_PROJECT_NAME} -> ${heptools_version}
  ${other_project} ${${other_project}_VERSION} -> ${${other_project}_heptools_version}

  You need to call cmake ${hint_message}
")
        endif()
        include_directories(${${other_project}_INCLUDE_DIRS})
        set(binary_paths ${${other_project}_BINARY_PATH})
        foreach(exported ${${other_project}_EXPORTED_SUBDIRS})
          list(FIND known_packages ${exported} is_needed)
          if(is_needed LESS 0)
            list(APPEND known_packages ${exported})
            message(STATUS "    importing ${exported}")
            get_filename_component(exported ${exported} NAME)
            include(${exported}Export)
          endif()
        endforeach()
        if(${other_project}_USES)
          list(INSERT ARGN_ 0 ${${other_project}_USES})
        endif()
      endif()
      #message(STATUS "know_packages (after ${other_project}) ${known_packages}")
    endif()

  endwhile()
endmacro()

#-------------------------------------------------------------------------------
# include_package_directories(Package1 [Package2 ...])
#
# Adde the include directories of each package to the include directories.
#-------------------------------------------------------------------------------
function(include_package_directories)
  #message(STATUS "include_package_directories(${ARGN})")
  foreach(package ${ARGN})
    # we need to ensure that the user can call this function also for directories
    if(TARGET ${package})
      get_target_property(to_incl ${package} SOURCE_DIR)
      if(to_incl)
        #message(STATUS "include_package_directories2 include_directories(${to_incl})")
        include_directories(${to_incl})
      endif()
    elseif(IS_DIRECTORY ${package})
      #message(STATUS "include_package_directories1 include_directories(${package})")
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
        #message(STATUS "include_package_directories3 include_directories(${${to_incl}})")
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
    if(TARGET ${package})
      set(collected ${collected} ${package})
      get_target_property(libs ${package} REQUIRED_LIBRARIES)
      if(libs)
        gaudi_resolve_link_libraries(libs ${libs})
        set(collected ${collected} ${libs})
      endif()
    elseif(EXISTS ${package})
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
  list(REMOVE_DUPLICATES collected)
  set(${variable} ${collected} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------
# gaudi_global_target_append(global_target local_target file1 [file2 ...])
# (macro)
#
# Adds local files as sources for the global target 'global_target' making it
# depend on the local target 'local_target'.
#-------------------------------------------------------------------------------
macro(gaudi_global_target_append global_target local_target)
  set_property(GLOBAL APPEND PROPERTY ${global_target}_SOURCES ${ARGN})
  set_property(GLOBAL APPEND PROPERTY ${global_target}_DEPENDS ${local_target})
endmacro()

#-------------------------------------------------------------------------------
# gaudi_global_target_get_info(global_target local_targets_var files_var)
# (macro)
#
# Put the information to configure the global target 'global_target' in the
# two variables local_targets_var and files_var.
#-------------------------------------------------------------------------------
macro(gaudi_global_target_get_info global_target local_targets_var files_var)
  get_property(${files_var} GLOBAL PROPERTY ${global_target}_SOURCES)
  get_property(${local_targets_var} GLOBAL PROPERTY ${global_target}_DEPENDS)
endmacro()


#-------------------------------------------------------------------------------
# gaudi_merge_files_append(merge_tgt local_target file1 [file2 ...])
#
# Add files to be included in the merge target 'merge_tgt', using 'local_target'
# as dependency trigger.
#-------------------------------------------------------------------------------
function(gaudi_merge_files_append merge_tgt local_target)
  gaudi_global_target_append(Merged${merge_tgt} ${local_target} ${ARGN})
endfunction()

#-------------------------------------------------------------------------------
# gaudi_merge_files(merge_tgt dest filename)
#
# Create a global target Merged${merge_tgt} that takes input files and dependencies
# from the packages (declared with gaudi_merge_files_append).
#-------------------------------------------------------------------------------
function(gaudi_merge_files merge_tgt dest filename)
  gaudi_global_target_get_info(Merged${merge_tgt} deps parts)
  if(parts)
    # create the targets
    set(output ${CMAKE_BINARY_DIR}/${dest}/${filename})
    add_custom_command(OUTPUT ${output}
                       COMMAND ${merge_cmd} ${parts} ${output}
                       DEPENDS ${parts})
    add_custom_target(Merged${merge_tgt} ALL DEPENDS ${output})
    # prepare the high level dependencies
    add_dependencies(Merged${merge_tgt} ${deps})
    # install rule for the merged DB
    install(FILES ${output} DESTINATION ${dest})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_generate_configurables(library)
#
# Internal function. Add the targets needed to produce the configurables for a
# module (component library).
#
# Note: see gaudi_install_python_modules for a description of how conflicts
#       between the installations of __init__.py are solved.
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
                  --xml ${env_xml}
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
  gaudi_merge_files_append(ConfDB ${library}Conf ${outdir}/${library}_confDb.py)
  #----Installation details-------------------------------------------------------
  install(FILES ${outdir}/${library}_confDb.py ${outdir}/${library}Conf.py
          DESTINATION python/${package})

  # Check if we need to install our __init__.py (i.e. it is not already installed
  # with the python modules).
  # Note: no need to do anything if we already have configurables
  get_property(has_configurables DIRECTORY PROPERTY has_configurables)
  if(NOT has_configurables)
    get_property(python_modules DIRECTORY PROPERTY has_python_modules)
    list(FIND python_modules ${package} got_pkg_module)
    if(got_pkg_module LESS 0)
      # we need to install our __init__.py
      install(FILES ${outdir}/__init__.py DESTINATION python/${package})
    endif()
  endif()

  # Property used to synchronize the installation of Python modules between
  # gaudi_generate_configurables and gaudi_install_python_modules.
  set_property(DIRECTORY APPEND PROPERTY has_configurables ${library})
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
                    --xml ${env_xml}
                ${genconfuser_cmd}
                  -r ${CMAKE_CURRENT_SOURCE_DIR}/python
                  -o ${outdir}/${package}_user_confDb.py
                  ${package} ${modules})
    install(FILES ${outdir}/${package}_user_confDb.py
            DESTINATION python/${package})
    gaudi_merge_files_append(ConfDB ${package}ConfUserDB ${outdir}/${package}_user_confDb.py)
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
      list(APPEND collected ${lib})
      get_property(req TARGET ${lib} PROPERTY REQUIRED_INCLUDE_DIRS)
      if(req)
        list(APPEND collected ${req})
      endif()
    endif()
  endforeach()
  if(collected)
    set(collected ${collected} ${${output}})
    list(REMOVE_DUPLICATES collected)
    set(${output} ${collected} PARENT_SCOPE)
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

  #message(STATUS "gaudi_common_add_build ${ARG_LINK_LIBRARIES}")
  # get the inherited include directories
  gaudi_get_required_include_dirs(ARG_INCLUDE_DIRS ${ARG_LINK_LIBRARIES})

  #message(STATUS "gaudi_common_add_build ${ARG_INCLUDE_DIRS}")
  # add the package includes to the current list
  include_package_directories(${ARG_INCLUDE_DIRS})

  #message(STATUS "gaudi_common_add_build ARG_LINK_LIBRARIES ${ARG_LINK_LIBRARIES}")
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

  gaudi_get_package_name(package)
  if(NOT ARG_NO_PUBLIC_HEADERS AND NOT ARG_PUBLIC_HEADERS)
    message(WARNING "Library ${library} (in ${package}) does not declare PUBLIC_HEADERS. Use the option NO_PUBLIC_HEADERS if it is intended.")
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
  set_target_properties(${library} PROPERTIES
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    REQUIRED_INCLUDE_DIRS "${ARG_INCLUDE_DIRS}"
    REQUIRED_LIBRARIES "${ARG_LINK_LIBRARIES}")
  set_property(GLOBAL APPEND PROPERTY LINKER_LIBRARIES ${library})

  if(TARGET ${library}Obj2doth)
    add_dependencies( ${library} ${library}Obj2doth)
  endif()
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION  ${lib})
  gaudi_export(LIBRARY ${library})
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

  gaudi_generate_rootmap(${library})
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
  gaudi_merge_files_append(DictRootmap ${dictionary}Gen ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname})

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
  gaudi_export(EXECUTABLE ${executable})

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
                 --xml ${env_xml}
               ${executable}${exec_suffix})
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
    find_package(QMTest QUIET)
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

  add_test(${package}.${name}
           ${env_cmd}
               ${extra_env}
               -p ${ld_library_path}=.
               -p ${ld_library_path}=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
               -p PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

               -p QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes
               -s GAUDI_QMTEST_HTML_OUTPUT=${CMAKE_BINARY_DIR}/test_results

               --xml ${env_xml}
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

#-------------------------------------------------------------------------------
# gaudi_install_python_modules()
#
# Declare that the subdirectory needs to install python modules.
# The hierarchy of directories and  files in the python directory will be
# installed.  If the first level of directories do not contain __init__.py, a
# warning is issued and an empty one will be installed.
#
# Note: We need to avoid conflicts with the automatic generated __init__.py for
#       configurables (gaudi_generate_configurables)
#       There are 2 cases:
#       * install_python called before genconf
#         we fill the list of modules to tell genconf not to install its dummy
#         version
#       * genconf called before install_python
#         we install on top of the one installed by genconf
# FIXME: it should be cleaner
#-------------------------------------------------------------------------------
function(gaudi_install_python_modules)
  install(DIRECTORY python/
          DESTINATION python
          FILES_MATCHING PATTERN "*.py")
  # check for the presence of the __init__.py's and install them if needed
  file(GLOB sub-dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} python/*)
  foreach(dir ${sub-dir})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} AND NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/__init__.py)
      message(WARNING "The file ${dir}/__ini__.py is missing. I shall install an empty one.")
      if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/__init__.py "# Empty file generated automatically\n")
      endif()
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
              DESTINATION ${CMAKE_INSTALL_PREFIX}/${dir})
    endif()
    # Add the Python module name to the list of provided ones.
    get_filename_component(modname ${dir} NAME)
    set_property(DIRECTORY APPEND PROPERTY has_python_modules ${modname})
  endforeach()
  gaudi_generate_confuserdb() # if there are Python modules, there may be ConfigurableUser's
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_install_scripts()
#
# Declare that the package needs to install the content of the 'scripts' directory.
#---------------------------------------------------------------------------------------------------
function(gaudi_install_scripts)
  install(DIRECTORY scripts/ DESTINATION scripts
          FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ
          PATTERN ".svn" EXCLUDE
          PATTERN "*~" EXCLUDE
          PATTERN "*.pyc" EXCLUDE)
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_install_joboptions(<files...>)
#
# Install the specified options files in the directory 'jobOptions/<package>'.
#---------------------------------------------------------------------------------------------------
function(gaudi_install_joboptions)
  gaudi_get_package_name(package)
  install(FILES ${ARGN} DESTINATION jobOptions/${package})
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_install_resources(<data files...> [DESTINATION subdir])
#
# Install the specified options files in the directory 'data/<package>[/subdir]'.
#---------------------------------------------------------------------------------------------------
function(gaudi_install_resources)
  CMAKE_PARSE_ARGUMENTS(ARG "" "DESTINATION" "" ${ARGN})

  gaudi_get_package_name(package)
  install(FILES ${ARG_UNPARSED_ARGUMENTS} DESTINATION data/${package}/${ARG_DESTINATION})
endfunction()

#---------------------------------------------------------------------------------------------------
# gaudi_project_version_header()
#
# Creates a header file with the version macros of the project.
#---------------------------------------------------------------------------------------------------
function(gaudi_project_version_header)
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
# gaudi_generate_rootmap(library)
#
# Create the .rootmap file needed by the plug-in system.
#---------------------------------------------------------------------------------------------------
function(gaudi_generate_rootmap library)
  find_package(ROOT QUIET)
  set(rootmapfile ${library}.rootmap)

  if(TARGET ${library})
    get_property(libname TARGET ${library} PROPERTY LOCATION)
  else()
    set(libname ${library})
  endif()
  add_custom_command(OUTPUT ${rootmapfile}
                     COMMAND ${env_cmd}
                       --xml ${env_xml}
		             ${ROOT_genmap_CMD} -i ${libname} -o ${rootmapfile}
                     DEPENDS ${library})
  add_custom_target(${library}Rootmap ALL DEPENDS ${rootmapfile})
  # Notify the project level target
  gaudi_merge_files_append(Rootmap ${library}Rootmap ${CMAKE_CURRENT_BINARY_DIR}/${library}.rootmap)
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
set(${CMAKE_PROJECT_NAME}_heptools_version ${heptools_version})

if(IS_DIRECTORY \${${CMAKE_PROJECT_NAME}_DIR}/InstallArea/\${LCG_platform}/cmake)
  list(INSERT CMAKE_MODULE_PATH 0 \${${CMAKE_PROJECT_NAME}_DIR}/InstallArea/\${LCG_platform}/cmake)
else()
  message(FATAL_ERROR \"Cannot find \${${CMAKE_PROJECT_NAME}_DIR}/InstallArea/\${LCG_platform}/cmake: platform not supported\")
endif()

set(${CMAKE_PROJECT_NAME}_VERSION ${CMAKE_PROJECT_VERSION})
set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set(${CMAKE_PROJECT_NAME}_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
set(${CMAKE_PROJECT_NAME}_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})

set(${CMAKE_PROJECT_NAME}_USES ${PROJECT_USES})

include(${CMAKE_PROJECT_NAME}PlatformConfig)
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

  string(REPLACE "\$" "\\\$" project_environment_string "${project_environment}")
  set(filename ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}PlatformConfig.cmake)
  file(WRITE ${filename}
"# File automatically generated: DO NOT EDIT.

# Get the exported informations about the targets
get_filename_component(_dir "\${CMAKE_CURRENT_LIST_FILE}" PATH)
#include(\${_dir}/${CMAKE_PROJECT_NAME}Exports.cmake)

# Set useful properties
get_filename_component(_dir "\${_dir}" PATH)
set(${CMAKE_PROJECT_NAME}_INCLUDE_DIRS \${_dir}/include)
set(${CMAKE_PROJECT_NAME}_LIBRARY_DIRS \${_dir}/lib)

set(${CMAKE_PROJECT_NAME}_BINARY_PATH \${_dir}/bin \${_dir}/scripts)
if(EXISTS \${_dir}/python.zip)
  set(${CMAKE_PROJECT_NAME}_PYTHON_PATH \${_dir}/python.zip)
else()
  set(${CMAKE_PROJECT_NAME}_PYTHON_PATH \${_dir}/python)
endif()

set(${CMAKE_PROJECT_NAME}_COMPONENT_LIBRARIES ${component_libraries})
set(${CMAKE_PROJECT_NAME}_LINKER_LIBRARIES ${linker_libraries})

set(${CMAKE_PROJECT_NAME}_ENVIRONMENT ${project_environment_string})

set(${CMAKE_PROJECT_NAME}_EXPORTED_SUBDIRS)
foreach(p ${packages})
  get_filename_component(pn \${p} NAME)
  if(EXISTS \${_dir}/cmake/\${pn}Export.cmake)
    set(${CMAKE_PROJECT_NAME}_EXPORTED_SUBDIRS \${${CMAKE_PROJECT_NAME}_EXPORTED_SUBDIRS} \${p})
  endif()
endforeach()
")

  install(FILES ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}PlatformConfig.cmake DESTINATION cmake)
endmacro()

#-------------------------------------------------------------------------------
# gaudi_env(<SET|PREPEND|APPEND|REMOVE> <var> <value> [...repeat...])
#
# Declare environment variables to be modified.
# Note: this is just a wrapper around set_property, the actual logic is in ...
#-------------------------------------------------------------------------------
function(gaudi_env)
  #message(STATUS "ARGN -> ${ARGN}")
  # ensure that the variables in the value are not expanded when passing the arguments
  #string(REPLACE "\$" "\\\$" _argn "${ARGN}")
  #message(STATUS "_argn -> ${_argn}")
  set_property(DIRECTORY APPEND PROPERTY ENVIRONMENT ${ARGN})
endfunction()


#-------------------------------------------------------------------------------
# _env_conf_pop_instruction(...)
#
# helper macro used by gaudi_generate_env_conf.
#-------------------------------------------------------------------------------
macro(_env_conf_pop_instruction instr lst)
  #message(STATUS "_env_conf_pop_instruction ${lst} => ${${lst}}")
  list(GET ${lst} 0 1 2 ${instr})
  #message(STATUS "_env_conf_pop_instruction ${instr} => ${${instr}}")
  #message(STATUS "_env_conf_pop_instruction ${lst} => ${${lst}}")
  list(REMOVE_AT ${lst} 0 1 2)
  #message(STATUS "_env_conf_pop_instruction ${lst} => ${${lst}}")
endmacro()

#-------------------------------------------------------------------------------
# _env_line(...)
#
# helper macro used by gaudi_generate_env_conf.
#-------------------------------------------------------------------------------
macro(_env_line cmd var val output)
  set(${output} "variable=\"${var}\">${val}</env:")
  if(${cmd} STREQUAL "SET")
    set(${output} "<env:set ${${output}}set>")
  elseif(${cmd} STREQUAL "PREPEND")
    set(${output} "<env:prepend ${${output}}prepend>")
  elseif(${cmd} STREQUAL "APPEND")
    set(${output} "<env:append ${${output}}append>")
  elseif(${cmd} STREQUAL "REMOVE")
    set(${output} "<env:remove ${${output}}remove>")
  else()
    message(FATAL_ERROR "Unknown environment command ${cmd}")
  endif()
endmacro()

#-------------------------------------------------------------------------------
# gaudi_generate_env_conf(filename <env description>)
#
# Generate the XML file describing the changes to the environment required by
# this project.
#-------------------------------------------------------------------------------
function(gaudi_generate_env_conf filename)
  set(data "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<env:config xmlns:env=\"EnvSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"EnvSchema EnvSchema.xsd \">\n")
  set(commands ${ARGN})
  #message(STATUS "start - ${commands}")
  while(commands)
    #message(STATUS "iter - ${commands}")
    _env_conf_pop_instruction(instr commands)
    # ensure that the variables in the value are not expanded when passing the arguments
    string(REPLACE "\$" "\\\$" instr "${instr}")
    _env_line(${instr} ln)
    set(data "${data}  ${ln}\n")
  endwhile()
  set(data "${data}</env:config>\n")
  message(STATUS "Generating ${filename}")
  file(WRITE ${filename} "${data}")
endfunction()

#-------------------------------------------------------------------------------
# gaudi_external_project_environment()
#
# Collect the environment details from the found packages and add them to the
# variable project_environment.
#-------------------------------------------------------------------------------
macro(gaudi_external_project_environment)
  message(STATUS "  environment for external packages")
  # collecting environment infos
  set(python_path)
  set(binary_path)
  set(environment)
  set(library_path2)

  if(CMAKE_HOST_UNIX)
    # Guess the LD_LIBRARY_PATH required by the compiler we use (only Unix).
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.so OUTPUT_VARIABLE cpplib)
    get_filename_component(cpplib ${cpplib} REALPATH)
    get_filename_component(cpplib ${cpplib} PATH)
    # Special hack for the way gcc is installed onf AFS at CERN.
    string(REPLACE "contrib/gcc" "external/gcc" cpplib ${cpplib})
    set(library_path2 ${cpplib})
  endif()

  get_property(packages_found GLOBAL PROPERTY PACKAGES_FOUND)
  #message("${packages_found}")
  foreach(pack ${packages_found})
    if(NOT pack STREQUAL GaudiProject)
      message(STATUS "    ${pack}")
      # this is needed to get the non-cache variables for the packages
      find_package(${pack} QUIET)

      if(pack STREQUAL PythonInterp OR pack STREQUAL PythonLibs)
        set(pack Python)
      endif()
      string(TOUPPER ${pack} _pack_upper)

      if(${_pack_upper}_EXECUTABLE)
        get_filename_component(bin_path ${${_pack_upper}_EXECUTABLE} PATH)
        list(APPEND binary_path ${bin_path})
      endif()

      list(APPEND binary_path   ${${pack}_BINARY_PATH} ${${_pack_upper}_BINARY_PATH})
      list(APPEND python_path   ${${pack}_PYTHON_PATH} ${${_pack_upper}_PYTHON_PATH})
      list(APPEND environment   ${${pack}_ENVIRONMENT} ${${_pack_upper}_ENVIRONMENT})
      list(APPEND library_path2 ${${pack}_LIBRARY_DIR} ${${pack}_LIBRARY_DIRS} ${${_pack_upper}_LIBRARY_DIR} ${${_pack_upper}_LIBRARY_DIRS})
    endif()
  endforeach()

  get_property(library_path GLOBAL PROPERTY LIBRARY_PATH)
  set(library_path ${library_path} ${library_path2})
  # Remove system libraries from the library_path
  list(REMOVE_ITEM library_path /usr/lib /lib /usr/lib64 /lib64 /usr/lib32 /lib32)

  foreach(var library_path python_path binary_path)
    if(${var})
      list(REMOVE_DUPLICATES ${var})
    endif()
  endforeach()

  foreach(val ${python_path})
    set(project_environment ${project_environment} PREPEND PYTHONPATH ${val})
  endforeach()

  foreach(val ${binary_path})
    set(project_environment ${project_environment} PREPEND PATH ${val})
  endforeach()

  foreach(val ${library_path})
    set(project_environment ${project_environment} PREPEND LD_LIBRARY_PATH ${val})
  endforeach()

  set(project_environment ${project_environment} ${environment})

endmacro()

#-------------------------------------------------------------------------------
# gaudi_export( (LIBRARY | EXECUTABLE)
#               <name> )
#
# Internal function used to export targets.
#-------------------------------------------------------------------------------
function(gaudi_export type name)
  set_property(DIRECTORY APPEND PROPERTY GAUDI_EXPORTED_${type} ${name})
endfunction()

#-------------------------------------------------------------------------------
# gaudi_generate_exports(subdirs...)
#
# Internal function that generate the export data.
#-------------------------------------------------------------------------------
macro(gaudi_generate_exports)
  foreach(package ${ARGN})
    # we do not use the "Hat" for the export names
    get_filename_component(pkgname ${package} NAME)
    get_property(exported_libs  DIRECTORY ${package} PROPERTY GAUDI_EXPORTED_LIBRARY)
    get_property(exported_execs DIRECTORY ${package} PROPERTY GAUDI_EXPORTED_EXECUTABLE)

    if (exported_libs OR exported_execs)
      set(pkg_exp_file ${pkgname}Export.cmake)

      message(STATUS "Generating ${pkg_exp_file}")
      set(pkg_exp_file ${CMAKE_CURRENT_BINARY_DIR}/${pkg_exp_file})

      file(WRITE ${pkg_exp_file}
"# File automatically generated: DO NOT EDIT.

# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
get_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)

")

      foreach(library ${exported_libs})
        file(APPEND ${pkg_exp_file} "add_library(${library} SHARED IMPORTED)\n")
        file(APPEND ${pkg_exp_file} "set_target_properties(${library} PROPERTIES\n")

        foreach(pn REQUIRED_INCLUDE_DIRS REQUIRED_LIBRARIES)
          get_property(prop TARGET ${library} PROPERTY ${pn})
          if (prop)
            file(APPEND ${pkg_exp_file} "  ${pn} \"${prop}\"\n")
          endif()
        endforeach()

        get_property(prop TARGET ${library} PROPERTY LOCATION)
        get_filename_component(prop ${prop} NAME)
        file(APPEND ${pkg_exp_file} "  IMPORTED_SONAME \"${prop}\"\n")
        file(APPEND ${pkg_exp_file} "  IMPORTED_LOCATION \"\${_IMPORT_PREFIX}/lib/${prop}\"\n")

        file(APPEND ${pkg_exp_file} "  )\n")
      endforeach()

      foreach(executable ${exported_execs})

        file(APPEND ${pkg_exp_file} "add_executable(${executable} IMPORTED)\n")
        file(APPEND ${pkg_exp_file} "set_target_properties(${executable} PROPERTIES\n")

        get_property(prop TARGET ${executable} PROPERTY LOCATION)
        get_filename_component(prop ${prop} NAME)
        file(APPEND ${pkg_exp_file} "  IMPORTED_LOCATION \"\${_IMPORT_PREFIX}/bin/${prop}\"\n")

        file(APPEND ${pkg_exp_file} "  )\n")
      endforeach()

    endif()
    install(FILES ${pkg_exp_file} DESTINATION cmake)
  endforeach()
endmacro()
