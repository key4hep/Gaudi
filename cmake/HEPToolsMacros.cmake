################################################################################
# HEP CMake toolchain
#-------------------------------------------------------------------------------
# The HEP CMake toolchain is required to build a project using the libraries and
# tools provided by SPI/SFT (a.k.a. LCGCMT).
#
# The variables used to tune the toolchain behavior are:
#
#  - BINARY_TAG: inferred from the system or from the environment (CMAKECONFIG,
#                CMTCONFIG), defines the target platform (by default is the same
#                as the host)
#  - LCG_SYSTEM: by default it is derived from BINARY_TAG, but it can be set
#                explicitly to a compatible supported platform if the default
#                one is not supported.
#                E.g.: if BINARY_TAG is x86_64-ubuntu1204-gcc46-opt, LCG_SYSTEM
#                      should be set to x86_64-slc6-gcc46.
################################################################################

################################################################################
# Define variables and location of the compiler.
macro(lcg_set_compiler flavor)
  if(NOT lcg_compiler_set)
    if(NOT ${flavor} STREQUAL "NATIVE")
      if(NOT HOST_BINARY_TAG_ARCH)
        include(BinaryTagUtils)
        get_host_binary_tag(HOST_BINARY_TAG)
        parse_binary_tag(HOST_BINARY_TAG)
      endif()
      set(version ${ARGV1})
      if(${flavor} MATCHES "^gcc|GNU$")
        set(compiler_root ${LCG_external}/gcc/${version}/${HOST_BINARY_TAG_ARCH}-${HOST_BINARY_TAG_OS})
        set(c_compiler_names lcg-gcc-${version})
        set(cxx_compiler_names lcg-g++-${version})
        set(fortran_compiler_names lcg-gfortran-${version})
      elseif(${flavor} STREQUAL "icc")
        # Note: icc must be in the path already because of the licensing
        set(compiler_root)
        set(c_compiler_names lcg-icc-${version} icc)
        set(cxx_compiler_names lcg-icpc-${version} icpc)
        set(fortran_compiler_names lcg-ifort-${version} ifort)
      elseif(${flavor} STREQUAL "clang")
        set(compiler_root ${LCG_external}/llvm/${version}/${HOST_BINARY_TAG_ARCH}-${HOST_BINARY_TAG_OS})
        set(c_compiler_names lcg-clang-${version} clang)
        set(cxx_compiler_names lcg-clang++-${version} clang++)
        # FIXME: clang does not come with a Fortran compiler
        set(fortran_compiler_names lcg-gfortran-4.8.1)
      else()
        message(FATAL_ERROR "Uknown compiler flavor ${flavor}.")
      endif()
      #message(STATUS "LCG_compiler(${ARGV}) -> '${c_compiler_names}' '${cxx_compiler_names}' '${fortran_compiler_names}' ${compiler_root}")
      find_program(CMAKE_C_COMPILER
                   NAMES ${c_compiler_names}
                   PATHS ${compiler_root}/bin
         DOC "C compiler")
      find_program(CMAKE_CXX_COMPILER
                   NAMES ${cxx_compiler_names}
                   PATHS ${compiler_root}/bin
         DOC "C++ compiler")
      find_program(CMAKE_Fortran_COMPILER
                   NAMES ${fortran_compiler_names}
                   PATHS ${compiler_root}/bin
         DOC "Fortran compiler")
      #message(STATUS "LCG_compiler(${ARGV}) -> ${CMAKE_C_COMPILER} ${CMAKE_CXX_COMPILER} ${CMAKE_Fortran_COMPILER}")
    endif()
    set(lcg_compiler_set "${ARGV}")
  else()
    if(NOT "${lcg_compiler_set}" STREQUAL "${ARGV}")
      message(WARNING "Attempt to change the compiler from ${lcg_compiler_set} to ${ARGV}")
    endif()
  endif()
endmacro()

################################################################################
# Add a specific compiler to the path
macro(lcg_set_lcg_system_compiler_path flavor)
  if(NOT ${flavor} STREQUAL "NATIVE")
    if(NOT HOST_BINARY_TAG_ARCH)
      include(BinaryTagUtils)
      get_host_binary_tag(HOST_BINARY_TAG)
      parse_binary_tag(HOST_BINARY_TAG)
    endif()
    set(version ${ARGV1})
    if(${flavor} MATCHES "^gcc|GNU$")
      set(lcg_system_compiler_path ${LCG_external}/gcc/${version}/${HOST_BINARY_TAG_ARCH}-${HOST_BINARY_TAG_OS})
    elseif(${flavor} STREQUAL "icc")
      # Note: icc must be in the path already because of the licensing
      set(lcg_system_compiler_path)
    elseif(${flavor} STREQUAL "clang")
      set(lcg_system_compiler_path ${LCG_external}/llvm/${version}/${HOST_BINARY_TAG_ARCH}-${HOST_BINARY_TAG_OS})
    else()
      message(FATAL_ERROR "Uknown compiler flavor ${flavor}.")
    endif()
  endif()
endmacro()

################################################################################
# Define variables and location of the compiler.
macro(_lcg_compiler id flavor version)
  if(NOT BINARY_TAG_COMP)
    include(BinaryTagUtils)
    parse_binary_tag()
  endif()
  if(${id} STREQUAL ${BINARY_TAG_COMP})
    lcg_set_compiler(${flavor} ${version})
  endif()
endmacro()

################################################################################
# Enable the correct compiler.
macro(lcg_common_compilers_definitions)
  if(NOT lcg_compiler_set)
    _lcg_compiler(gcc43 gcc 4.3.6)
    _lcg_compiler(gcc46 gcc 4.6.3)
    _lcg_compiler(gcc47 gcc 4.7.2)
    _lcg_compiler(gcc48 gcc 4.8.1)
    _lcg_compiler(gcc49 gcc 4.9.1)
    _lcg_compiler(clang30 clang 3.0)
    _lcg_compiler(clang32 clang 3.2)
    _lcg_compiler(clang33 clang 3.3)
    _lcg_compiler(clang34 clang 3.4)
    _lcg_compiler(clang35 clang 3.5)
    _lcg_compiler(clang37 clang 3.7)
  endif()
endmacro()

################################################################################
# Define variables for an LCG external.
macro(lcg_set_external name hash version dir)
    set(${name}_config_version ${version} CACHE STRING "Version of ${name}" FORCE)
    mark_as_advanced(${name}_config_version)
    set(${name}_native_version ${${name}_config_version})
    if(NOT IS_ABSOLUTE ${dir})
      set(${name}_home ${LCG_releases}/${dir})
    else()
      set(${name}_home ${dir})
    endif()

    if("${name}" MATCHES "ROOT|COOL|CORAL|RELAX|LCGCMT")
        #message(STATUS "AA Project ${name} -> ${${name}_config_version}")
        get_filename_component(${name}_base ${${name}_home} PATH)
        if(${name} STREQUAL ROOT)
          # ROOT is special
          set(ROOT_home ${ROOT_home} ${ROOT_home}/root)
        endif()
        list(APPEND LCG_projects ${name})
    elseif("${name}" STREQUAL "cmaketools" AND
           NOT EXISTS "${cmaketools_home}/CMakeToolsConfig.cmake")
        # ignore old versions of cmaketools
    elseif("${name}" STREQUAL "Qt5" AND
           CMAKE_VERSION VERSION_LESS "2.12")
        # we cannot mix Qt5 and Qt4 with CMake < 2.12, so we ignore Qt5
    else()
        #message(STATUS "External ${name} -> ${${name}_config_version}")
        list(APPEND LCG_externals ${name})
    endif()
endmacro()

################################################################################
# Find common programs that a toolchain should define (not mandatory).
macro(lcg_find_common_tools)
  #=============================================================================
  # Path to programs that a toolchain should define (not mandatory).
  #=============================================================================
  if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_program(CMAKE_AR       ar       )
    find_program(CMAKE_LINKER   ld       )
    find_program(CMAKE_NM       nm       )
    find_program(CMAKE_OBJCOPY  objcopy  )
    find_program(CMAKE_OBJDUMP  objdump  )
    find_program(CMAKE_RANLIB   ranlib   )
    find_program(CMAKE_STRIP    strip    )
    mark_as_advanced(CMAKE_AR CMAKE_LINKER CMAKE_NM CMAKE_OBJCOPY CMAKE_OBJDUMP
                     CMAKE_RANLIB CMAKE_STRIP)
  endif()
endmacro()

################################################################################
# Define the search paths from the configured versions
macro(lcg_prepare_paths)
  #===============================================================================
  # Derived variables
  #===============================================================================
  if(Python_config_version)
    string(REGEX MATCH "[0-9]+\\.[0-9]+" Python_config_version_twodigit ${Python_config_version})
    set(Python_ADDITIONAL_VERSIONS ${Python_config_version_twodigit})
  endif()

  # Note: this is needed because FindBoost.cmake requires both if the patch version is 0.
  if (Boost_config_version)
    string(REGEX MATCH "[0-9]+\\.[0-9]+" Boost_config_version_twodigit ${Boost_config_version})
    set(Boost_ADDITIONAL_VERSIONS ${Boost_config_version} ${Boost_config_version_twodigit})
  endif()

  # Useful for RedHat-derived platforms
  set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

  #===============================================================================
  # Special cases that require a special treatment
  #===============================================================================
  if (Boost_config_version)
    if(NOT APPLE)
      # FIXME: this should be automatic... see FindBoost.cmake documentation
      # Get Boost compiler id from LCG_SYSTEM or BINARY_TAG
      if(LCG_SYSTEM)
        string(REGEX MATCHALL "[^-]+" out ${LCG_SYSTEM})
        list(GET out 2 syscomp)
        set(Boost_COMPILER -${syscomp})
      else()
        if(NOT BINARY_TAG_COMP)
          include(BinaryTagUtils)
          parse_binary_tag()
        endif()
        set(Boost_COMPILER -${BINARY_TAG_COMP})
      endif()
      #message(STATUS "Boost compiler suffix: ${Boost_COMPILER}")
    endif()
    set(Boost_NO_BOOST_CMAKE ON)
    set(Boost_NO_SYSTEM_PATHS ON)
  endif()

  # Required if both Qt3 and Qt4 are available.
  if(Qt_config_version)
    string(REGEX MATCH "[0-9]+" _qt_major_version ${Qt_config_version})
    set(DESIRED_QT_VERSION ${_qt_major_version} CACHE STRING "Pick a version of QT to use: 4 or 5")
    mark_as_advanced(DESIRED_QT_VERSION)
    if(Qt5_config_version AND NOT CMAKE_VERSION VERSION_LESS "2.12")
      # Required if both Qt(4) and Qt5 are available.
      if(EXISTS "${Qt_home}/bin/qmake")
        set(QT_QMAKE_EXECUTABLE "${Qt_home}/bin/qmake" CACHE INTERNAL "")
      endif()
      set(CMAKE_PREFIX_PATH ${Qt5_home}/lib/cmake ${CMAKE_PREFIX_PATH})
    endif()
  endif()

  # This is not really needed because Xerces has its own version macro, but it was
  # added at some point, so it is kept for backward compatibility.
  #add_definitions(-DXERCESC_GE_31)

  #===============================================================================
  # Construct the actual PREFIX and INCLUDE PATHs
  #===============================================================================
  foreach(name ${LCG_projects})
    list(APPEND LCG_PREFIX_PATH ${${name}_home})
    list(APPEND LCG_INCLUDE_PATH ${${name}_base}/include)
    # We need to add python to the include path because it's the only
    # way to search for a (generic) file.
    foreach(h ${${name}_home})
      list(APPEND LCG_INCLUDE_PATH ${h}/python)
    endforeach()
  endforeach()
  # Add the LCG externals dirs to the search paths.
  foreach(name ${LCG_externals})
    list(APPEND LCG_PREFIX_PATH ${${name}_home})
  endforeach()

  # AIDA is special
  list(APPEND LCG_INCLUDE_PATH ${LCG_external}/${AIDA_directory_name}/${AIDA_config_version}/share/src/cpp)

  set(CMAKE_PREFIX_PATH ${LCG_PREFIX_PATH} ${CMAKE_PREFIX_PATH})
  set(CMAKE_INCLUDE_PATH ${LCG_INCLUDE_PATH} ${CMAKE_INCLUDE_PATH})

  #message(STATUS "LCG_PREFIX_PATH: ${LCG_PREFIX_PATH}")
  #message(STATUS "LCG_INCLUDE_PATH: ${LCG_INCLUDE_PATH}")
endmacro()
