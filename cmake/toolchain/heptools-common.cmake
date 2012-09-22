# Deduce the LCG configuration tag from the system
function(get_configuration_tag tag)
  #---arch--------------
  if(UNIX)
    execute_process(COMMAND uname -p OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    set(arch $ENV{PROCESSOR_ARCHITECTURE})
  endif()
  #---os----------------
  if(APPLE)
    set(os mac)
    execute_process(COMMAND sw_vers "-productVersion"
                    COMMAND cut -d . -f 1-2
                    OUTPUT_VARIABLE osvers OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE "." "" osvers ${osvers})
  elseif(WIN32)
    set(os winxp)
    set(osvers)
  else()
    execute_process(COMMAND cat /etc/issue OUTPUT_VARIABLE issue OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(issue MATCHES Ubuntu)
      set(os ubuntu)
      string(REGEX REPLACE ".*Ubuntu ([0-9]+)[.].*" "\\1" osvers "${issue}")
    elseif(issue MATCHES SLC)
      set(os slc)
      string(REGEX REPLACE ".*release ([0-9]+)[.].*" "\\1" osvers "${issue}")
    else()
      set(os linux)
      set(osvers)
    endif()
  endif()
  #---compiler-----------
  if(WIN32)
    find_program(heptools_guessed_compiler cl gcc)
  else()
    find_program(heptools_guessed_compiler gcc icc)
  endif()
  if(heptools_guessed_compiler MATCHES /cl)
    set(compiler vc)
    execute_process(COMMAND ${heptools_guessed_compiler} ERROR_VARIABLE versioninfo OUTPUT_VARIABLE out)
    string(REGEX REPLACE ".*Version ([0-9]+)[.].*" "\\1" cvers "${versioninfo}")
    math(EXPR cvers "${cvers} - 6")
  elseif(heptools_guessed_compiler MATCHES /gcc)
    set(compiler gcc)
    execute_process(COMMAND ${heptools_guessed_compiler} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
    list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
    set(cvers ${GCC_MAJOR}${GCC_MINOR})
  else()
    set(compiler)
    set(cvers)
  endif()
  mark_as_advanced(heptools_guessed_compiler)
  set(${tag} ${arch}-${os}${osvers}-${compiler}${cvers}-opt PARENT_SCOPE)
endfunction()


if(NOT DEFINED BINARY_TAG)
  # This toolchain requires the environment variables CMTCONFIG or CMAKECONFIG
  if(DEFINED ENV{CMAKECONFIG})
    message(STATUS "binary tag from CMAKECONFIG")
    set(BINARY_TAG $ENV{CMAKECONFIG})
  elseif(DEFINED ENV{CMTCONFIG})
    message(STATUS "binary tag from CMTCONFIG")
    set(BINARY_TAG $ENV{CMTCONFIG})
  else()
    message(STATUS "Configuration tag not specified: trying to deduce it.")
    get_configuration_tag(BINARY_TAG)
  endif()
  set(BINARY_TAG ${BINARY_TAG} CACHE INTERNAL "Id of the platform")
  message(STATUS "Using BINARY_TAG=${BINARY_TAG}")
endif()

# Split the binary tag
string(REGEX MATCHALL "[^-]+" out ${BINARY_TAG})
list(GET out 0 arch)
list(GET out 1 os)
list(GET out 2 comp)
list(GET out 3 type)

# architecture
set(CMAKE_SYSTEM_PROCESSOR ${arch})

# system name
if(os STREQUAL "winxp")
  set(CMAKE_SYSTEM_NAME Windows)
elseif(os MATCHES "mac.*")
  set(CMAKE_SYSTEM_NAME Darwin)
elseif(os MATCHES "slc[456]")
  set(CMAKE_SYSTEM_NAME Linux)
else()
  message(FATAL_ERROR "OS ${os} is not supported.")
endif()

# build type
if(type STREQUAL "opt")
  set(type Release)
elseif(type STREQUAL "dbg")
  set(type Debug)
elseif(type STREQUAL "cov")
  set(type Coverage)
else()
  message(FATAL_ERROR "Build flavour ${type} not supported.")
endif()
set(CMAKE_BUILD_TYPE ${type} CACHE STRING
    "Choose the type of build, options are: None Debug Release Coverage Profile RelWithDebInfo MinSizeRel.")

# Platform IDs
set(LCG_platform ${BINARY_TAG} CACHE STRING "Platform ID for the AA project binaries.")
set(LCG_system ${arch}-${os}-${comp}-opt CACHE STRING "Platform ID for the external libraries.")

# LCG location
if(NOT heptools_version)
  message(FATAL_ERROR "Variable heptools_version not defined. It must be defined before including heptools-common.cmake")
endif()

find_path(LCG_releases LCGCMT/LCGCMT_${heptools_version} ENV CMTPROJECTPATH)
if(NOT LCG_releases)
  message(FATAL_ERROR "Cannot find location of LCGCMT ${heptools_version}")
endif()
# define location of externals
get_filename_component(_lcg_rel_type ${LCG_releases} NAME)
if(_lcg_rel_type STREQUAL "external")
  set(LCG_external ${LCG_releases})
else()
  get_filename_component(LCG_external ${LCG_releases}/../../external ABSOLUTE)
endif()

# Flag the LCG internal cached variables as "advanced".
mark_as_advanced(LCG_platform LCG_system)

# Define the variables and search paths for AA projects
macro(LCG_AA_project name version)
  set(${name}_config_version ${version})
  set(${name}_native_version ${version})
  set(${name}_base ${LCG_releases}/${name}/${${name}_native_version})
  set(${name}_home ${${name}_base}/${LCG_platform})
  if(${name} STREQUAL ROOT)
    # ROOT is special
    set(ROOT_home ${ROOT_home}/root)
  endif()
  list(APPEND LCG_projects ${name})
endmacro()

macro(LCG_compiler id flavor version)
  if(${id} STREQUAL ${comp})
    if(${flavor} STREQUAL "gcc")
      set(compiler_root ${LCG_external}/${flavor}/${version}/${arch}-${os})
      set(CMAKE_C_COMPILER   ${compiler_root}/bin/lcg-gcc-${version})
      set(CMAKE_CXX_COMPILER ${compiler_root}/bin/lcg-g++-${version})
    elseif(${flavor} STREQUAL "clang")
      set(compiler_root ${LCG_external}/llvm/${version}/${arch}-${os})
      set(CMAKE_C_COMPILER   ${compiler_root}/bin/clang)
      set(CMAKE_CXX_COMPILER ${compiler_root}/bin/clang++)
    else()
      message(FATAL_ERROR "Uknown compiler flavor ${flavor}.")
    endif()
  endif()
endmacro()


# Define the variables for external projects
# Usage:
#   LCG_external_package(<Package> <version> [<directory name>])
# Examples:
#   LCG_external_package(Boost 1.44.0)
#   LCG_external_package(CLHEP 1.9.4.7 clhep)
macro(LCG_external_package name version)
  set(${name}_config_version ${version})
  set(${name}_native_version ${version})
  if(${ARGC} GREATER 2)
    set(${name}_directory_name ${ARGV2})
  else()
    set(${name}_directory_name ${name})
  endif()
  list(APPEND LCG_externals ${name})
endmacro()

# Define the search paths from the configured versions
macro(LCG_prepare_paths)
  #===============================================================================
  # Derived variables
  #===============================================================================
  string(REGEX MATCH "[0-9]+\\.[0-9]+" Python_config_version_twodigit ${Python_config_version})
  set(Python_ADDITIONAL_VERSIONS ${Python_config_version_twodigit})

  # Note: this is needed because FindBoost.cmake requires both if the patch version is 0.
  string(REGEX MATCH "[0-9]+\\.[0-9]+" Boost_config_version_twodigit ${Boost_config_version})
  set(Boost_ADDITIONAL_VERSIONS ${Boost_config_version} ${Boost_config_version_twodigit})

  #===============================================================================
  # Special cases that require a special treatment
  #===============================================================================
  set(Boost_native_version ${Boost_config_version}_python${Python_config_version_twodigit})
  if(NOT APPLE)
    # FIXME: this should be automatic... see FindBoost.cmake documentation
    # Get Boost compiler id from LCG_system
    string(REGEX MATCHALL "[^-]+" out ${LCG_system})
    list(GET out 2 syscomp)
    set(Boost_COMPILER -${syscomp})
  endif()
  set(Boost_NO_BOOST_CMAKE ON)
  set(Boost_NO_SYSTEM_PATHS ON)

  set(pyanalysis_native_version ${pyanalysis_config_version}_python${Python_config_version_twodigit})

  set(pytools_native_version ${pytools_config_version}_python${Python_config_version_twodigit})

  set(QMtest_native_version ${QMtest_config_version}_python${Python_config_version_twodigit})

  if(comp STREQUAL clang30)
    set(GCCXML_CXX_COMPILER gcc CACHE STRING "Compiler that GCCXML must use.")
  endif()

  # This is not really needed because Xerces has its own version macro, but it was
  # added at some point, so it is kept for backward compatibility.
  #add_definitions(-DXERCESC_GE_31)

  #===============================================================================
  # Construct the actual PREFIX and INCLUDE PATHs
  #===============================================================================
  # Define the _home variables (not cached)
  foreach(name ${LCG_externals})
    set(${name}_home ${LCG_external}/${${name}_directory_name}/${${name}_native_version}/${LCG_system})
  endforeach()

  if(NOT DEFINED LCG_PREFIX_PATH)
    foreach(name ${LCG_projects})
      list(APPEND LCG_PREFIX_PATH ${${name}_home})
      # We need to add python to the include path because it's the only
      # way to search for a (generic) file.
      list(APPEND LCG_INCLUDE_PATH ${${name}_base}/include ${${name}_home}/python)
    endforeach()
    # Add the LCG externals dirs to the search paths.
    foreach(name ${LCG_externals})
      list(APPEND LCG_PREFIX_PATH ${${name}_home})
    endforeach()

    # AIDA is special
    list(APPEND LCG_INCLUDE_PATH ${LCG_external}/${AIDA_directory_name}/${AIDA_native_version}/share/src/cpp)

    set(LCG_PREFIX_PATH ${LCG_PREFIX_PATH} CACHE INTERNAL "Search path for external libraries")
    set(LCG_INCLUDE_PATH ${LCG_INCLUDE_PATH} CACHE INTERNAL "Search path for files")
  endif()

  set(CMAKE_PREFIX_PATH ${LCG_PREFIX_PATH} ${CMAKE_PREFIX_PATH})
  set(CMAKE_INCLUDE_PATH ${LCG_INCLUDE_PATH} ${CMAKE_INCLUDE_PATH})

endmacro()
