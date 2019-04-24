# parse binary tag
include(BinaryTagUtils)
if(NOT BINARY_TAG_COMP)
  parse_binary_tag()
endif()
if(NOT HOST_BINARY_TAG_ARCH)
  get_host_binary_tag(HOST_BINARY_TAG)
  parse_binary_tag(HOST_BINARY_TAG)
endif()

check_compiler()


# Convert BINARY_TAG_TYPE to CMAKE_BUILD_TYPE
if(BINARY_TAG_TYPE STREQUAL "opt")
  set(_BT_CMAKE_BUILD_TYPE Release)
elseif(BINARY_TAG_TYPE MATCHES "^dbg|do0$")
  set(_BT_CMAKE_BUILD_TYPE Debug)
elseif(BINARY_TAG_TYPE STREQUAL "cov")
  set(_BT_CMAKE_BUILD_TYPE Coverage)
elseif(BINARY_TAG_TYPE STREQUAL "pro")
  set(_BT_CMAKE_BUILD_TYPE Profile)
#elseif(BINARY_TAG_TYPE STREQUAL "o2g")
#  set(CMAKE_BUILD_TYPE RelWithDebInfo)
#elseif(BINARY_TAG_TYPE STREQUAL "min")
#  set(CMAKE_BUILD_TYPE MinSizeRel)
else()
  message(FATAL_ERROR "BINARY_TAG build type ${BINARY_TAG_TYPE} not supported.")
endif()
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${_BT_CMAKE_BUILD_TYPE} CACHE STRING
      "Choose the type of build, options are: Release, Debug, Coverage, Profile, RelWithDebInfo, MinSizeRel."
      FORCE)
else()
  if(NOT _BT_CMAKE_BUILD_TYPE STREQUAL CMAKE_BUILD_TYPE)
    message(WARNING "CMAKE_BUILD_TYPE set to ${CMAKE_BUILD_TYPE}, but BINARY_TAG build type ${BINARY_TAG_TYPE} implies ${_BT_CMAKE_BUILD_TYPE}")
  endif()
endif()


# Report the platform ids.
message(STATUS "Target system:    ${BINARY_TAG}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Host system:      ${HOST_BINARY_TAG}")
message(STATUS "LCG system:       ${LCG_SYSTEM}")


# define a minimun default version
set(GAUDI_CXX_STANDARD_DEFAULT "c++14")
# overriddend depending on the compiler
if (BINARY_TAG_COMP_NAME STREQUAL "clang" AND NOT BINARY_TAG_COMP_VERSION VERSION_LESS "3.9")
  set(GAUDI_CXX_STANDARD_DEFAULT "c++1z")
elseif(BINARY_TAG_COMP_NAME STREQUAL "gcc")
  # Special defaults
  if (NOT BINARY_TAG_COMP_VERSION VERSION_LESS "7.0")
    set(GAUDI_CXX_STANDARD_DEFAULT "c++17")
  endif()
endif()
# special for GaudiHive
set(GAUDI_CPP11_DEFAULT ON)

# Detect the version of the C++ standard used to compile ROOT
# and, if found, use it as default standard for the build.
if(NOT ROOT_CXX_STANDARD)
  find_package(ROOT QUIET)
  if(ROOT_FOUND)
    file(STRINGS ${ROOT_INCLUDE_DIR}/RConfigure.h _RConfigure REGEX "define R__")
    foreach(rconfig_line IN LISTS _RConfigure)
      if(rconfig_line MATCHES "R__USE_CXX([^ ]+)")
        set(ROOT_CXX_STANDARD "c++${CMAKE_MATCH_1}")
      endif()
    endforeach()
    if(ROOT_CXX_STANDARD)
      set(ROOT_CXX_STANDARD "${ROOT_CXX_STANDARD}" CACHE INTERNAL
          "C++ standard used by ROOT")
      mark_as_advanced(ROOT_CXX_STANDARD)
    endif()
    set(_RConfigure)
    set(rconfig_line)
  endif()
endif()
if(ROOT_CXX_STANDARD)
  set(GAUDI_CXX_STANDARD_DEFAULT "${ROOT_CXX_STANDARD}")
endif()

#--- Gaudi Build Options -------------------------------------------------------
# Build options that map to compile time features
#
option(GAUDI_V21
       "disable backward compatibility hacks (implies all G21_* options)"
       OFF)
option(G21_HIDE_SYMBOLS
       "enable explicit symbol visibility on gcc-4"
       OFF)
option(G21_NEW_INTERFACES
       "disable backward-compatibility hacks in IInterface and InterfaceID"
       OFF)
option(G21_NO_DEPRECATED
       "remove deprecated methods and functions"
       OFF)
option(G22_NEW_SVCLOCATOR
       "use (only) the new interface of the ServiceLocator"
       OFF)
option(GAUDI_V22
       "enable some API extensions"
       OFF)

if (BINARY_TAG_COMP_NAME STREQUAL "gcc" AND BINARY_TAG_COMP_VERSION VERSION_GREATER "5.0")
  option(GAUDI_SUGGEST_OVERRIDE "enable warnings for missing override keyword" ON)
endif()


string(COMPARE EQUAL "${BINARY_TAG_TYPE}" "do0" GAUDI_SLOW_DEBUG_DEFAULT)
option(GAUDI_SLOW_DEBUG
       "turn off all optimizations in debug builds"
       ${GAUDI_SLOW_DEBUG_DEFAULT})

option(GAUDI_DIAGNOSTICS_COLOR "enable colors in compiler diagnostics" OFF)

# set optimization flags (_opt_level_* and _opt_ext_*)
# - default optimization levels
set(_opt_level_RELEASE "-O3")
set(_opt_ext_RELEASE "-DNDEBUG")
if(NOT GAUDI_SLOW_DEBUG AND BINARY_TAG_COMP_NAME STREQUAL "gcc")
  # Use -Og with Debug builds in gcc (if not disabled)
  set(_opt_level_DEBUG "-Og")
else()
  set(_opt_level_DEBUG "-O0")
endif()
set(_opt_ext_DEBUG "-g")
# (RelWithDebInfo shares the flags with Release)
set(_opt_level_RELWITHDEBINFO "${_opt_level_RELEASE}")
set(_opt_ext_RELWITHDEBINFO "${_opt_ext_RELEASE} -g")

# Sanitizer options
# http://clang.llvm.org/docs/AddressSanitizer.html
# http://clang.llvm.org/docs/LeakSanitizer.html
# http://clang.llvm.org/docs/ThreadSanitizer.html
# http://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
set(GAUDI_asan_FLAGS "-fsanitize=address -fsanitize-recover=all -fno-omit-frame-pointer -lasan"
    CACHE STRING "Build options for AddressSanitizer")
set(GAUDI_lsan_FLAGS "-fsanitize=leak -fsanitize-recover=all -fno-omit-frame-pointer -llsan"
    CACHE STRING "Build options for LeakSanitizer")
set(GAUDI_tsan_FLAGS "-fsanitize=thread -fsanitize-recover=all -fno-omit-frame-pointer -ltsan"
    CACHE STRING "Build options for ThreadSanitizer")
set(GAUDI_ubsan_FLAGS "-fsanitize=undefined -fsanitize-recover=all -fno-omit-frame-pointer -lubsan"
    CACHE STRING "Build options for UndefinedSanitizer")
set(GAUDI_DEFAULT_SANITIZER)

# - parse subtype flags
string(TOUPPER "${CMAKE_BUILD_TYPE}" _up_bt)
foreach(_subtype ${BINARY_TAG_SUBTYPE})
  if(_subtype MATCHES "^[oO]([0-3sg]|fast)$")
    set(_opt_level_${_up_bt} "-O${CMAKE_MATCH_1}")
    #message(STATUS "setting _opt_level_${_up_bt} -> ${_opt_level_${_up_bt}}")
  elseif(_subtype STREQUAL "g")
    set(_opt_ext_${_up_bt} "${_opt_ext_${_up_bt}} -g")
  elseif(_subtype STREQUAL "cov")
    set(_opt_ext_${_up_bt} "${_opt_ext_${_up_bt}} --coverage")
  elseif(_subtype MATCHES "^(a|l|t|ub)san$")
    set(GAUDI_DEFAULT_SANITIZER ${_subtype})
  endif()
endforeach()

set(GAUDI_USE_SANITIZER ${GAUDI_DEFAULT_SANITIZER} CACHE STRING "Enabled given sanitizer")
if (GAUDI_USE_SANITIZER)
  if(NOT GAUDI_USE_SANITIZER MATCHES "^(a|l|t|ub)san$")
    message(FATAL_ERROR "Invalid sanitizer name: ${GAUDI_USE_SANITIZER}")
  else()
    set(SANITIZER_ENABLED "lib${GAUDI_USE_SANITIZER}.so")
    set(_opt_ext_${_up_bt} "${_opt_ext_${_up_bt}} ${GAUDI_${GAUDI_USE_SANITIZER}_FLAGS}")
  endif()
endif()

if(_opt_level_${_up_bt})
  message(STATUS "Optimization:     ${_opt_level_${_up_bt}} ${_opt_ext_${_up_bt}}")
endif()

# extra flags to enable/disable specific instruction sets
set(GAUDI_ARCH_DEFAULT)
if(BINARY_TAG_MICROARCH)
  set(GAUDI_ARCH_DEFAULT ${BINARY_TAG_MICROARCH})
elseif(BINARY_TAG_COMP_NAME STREQUAL "gcc" AND BINARY_TAG_COMP_VERSION VERSION_GREATER "5.0" AND
   BINARY_TAG_ARCH STREQUAL "x86_64")
  # Special case: x86_64-*-gcc6 or higher is equivalent to x86_64+sse4.2-*
  set(GAUDI_ARCH_DEFAULT "sse4.2")
else()
  # if no extra flags and not special case, compare host and target architecture
  # (BTU_KNOWN_x86_ARCHS is list ordered such that later entries can build earliler entries)
  list(FIND BTU_KNOWN_x86_ARCHS ${BINARY_TAG_ARCH} _target_idx)
  list(FIND BTU_KNOWN_x86_ARCHS ${HOST_BINARY_TAG_ARCH} _host_idx)
  if (_host_idx LESS _target_idx)
    message(FATAL_ERROR "Cannot build for '${BINARY_TAG_ARCH}' on '${HOST_BINARY_TAG_ARCH}'.")
  endif()
endif()
set(GAUDI_ARCH "${GAUDI_ARCH_DEFAULT}"
    CACHE STRING "Which architecture-specific optimizations to use")

if(DEFINED GAUDI_CPP11)
  message(WARNING "GAUDI_CPP11 is an obsolete option, use GAUDI_CXX_STANDARD=c++11 instead")
endif()

set(GAUDI_CXX_STANDARD "${GAUDI_CXX_STANDARD_DEFAULT}"
    CACHE STRING "Version of the C++ standard to be used.")
if(ROOT_CXX_STANDARD AND NOT ROOT_CXX_STANDARD STREQUAL GAUDI_CXX_STANDARD)
  message(WARNING "Requested ${GAUDI_CXX_STANDARD} but ROOT was compiled with ${ROOT_CXX_STANDARD}")
endif()

# If modern c++ and gcc >= 5.1 and requested, use old ABI compatibility
if((NOT GAUDI_CXX_STANDARD STREQUAL "c++98") AND
   (BINARY_TAG_COMP_NAME STREQUAL "gcc" AND NOT BINARY_TAG_COMP_VERSION VERSION_LESS "5.1") AND
   GAUDI_GCC_OLD_ABI)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

# summary of options affecting cached build flags
set(GAUDI_FLAGS_OPTIONS "${BINARY_TAG};${GAUDI_ARCH};${GAUDI_SLOW_DEBUG};${GAUDI_SUGGEST_OVERRIDE}")

#--- Compilation Flags ---------------------------------------------------------
if(NOT GAUDI_FLAGS_SET EQUAL GAUDI_FLAGS_OPTIONS)
  #message(STATUS "Setting cached build flags")
  if(MSVC90)

    set(CMAKE_CXX_FLAGS_DEBUG "/D_NDEBUG /MD /Zi /Ob0 /Od /RTC1"
        CACHE STRING "Flags used by the compiler during debug builds."
        FORCE)
    set(CMAKE_C_FLAGS_DEBUG "/D_NDEBUG /MD /Zi /Ob0 /Od /RTC1"
        CACHE STRING "Flags used by the compiler during debug builds."
        FORCE)

  else()
    # special architecture flags
    string(REPLACE "_" "-" _gcc_arch_name ${BINARY_TAG_ARCH})
    set(arch_opts "-march=${_gcc_arch_name}")
    foreach(_arch_opt ${GAUDI_ARCH})
      if(_arch_opt STREQUAL "native")
        message(FATAL_ERROR "you must use 'native-${BINARY_TAG_OS}-${BINARY_TAG_COMP}-${BINARY_TAG_TYPE}'' instead of '${BINARY_TAG_ARCH}+native-${BINARY_TAG_OS}-${BINARY_TAG_COMP}-${BINARY_TAG_TYPE}'")
      endif()
      set(arch_opts "${arch_opts} -m${_arch_opt}")
    endforeach()
    # Common compilation flags
    set(CMAKE_CXX_FLAGS
        "${arch_opts} -fmessage-length=0 -pipe -Wall -Wextra -Werror=return-type -pthread -pedantic -Wwrite-strings -Wpointer-arith -Woverloaded-virtual -Wnon-virtual-dtor"
        CACHE STRING "Flags used by the compiler during all build types."
        FORCE)
    set(CMAKE_C_FLAGS
        "${arch_opts} -fmessage-length=0 -pipe -Wall -Wextra -Werror=return-type -pthread -pedantic -Wwrite-strings -Wpointer-arith"
        CACHE STRING "Flags used by the compiler during all build types."
        FORCE)
    set(CMAKE_Fortran_FLAGS
        "${arch_opts} -fmessage-length=0 -pipe -Wall -Wextra -Werror=return-type -pthread -pedantic -fsecond-underscore"
        CACHE STRING "Flags used by the compiler during all build types."
        FORCE)

    if (BINARY_TAG_COMP_NAME STREQUAL "gcc" AND BINARY_TAG_COMP_VERSION VERSION_GREATER "5.0" AND GAUDI_SUGGEST_OVERRIDE)
        set(CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} -Wsuggest-override"
            CACHE STRING "Flags used by the compiler during all build types."
            FORCE)
    endif()

    # Build type compilation flags
    set(CMAKE_CXX_FLAGS_RELEASE "${_opt_level_RELEASE} ${_opt_ext_RELEASE}"
        CACHE STRING "Flags used by the compiler during release builds."
        FORCE)
    set(CMAKE_C_FLAGS_RELEASE "${_opt_level_RELEASE} ${_opt_ext_RELEASE}"
        CACHE STRING "Flags used by the compiler during release builds."
        FORCE)
    set(CMAKE_Fortran_FLAGS_RELEASE "${_opt_level_RELEASE} ${_opt_ext_RELEASE}"
        CACHE STRING "Flags used by the compiler during release builds."
        FORCE)

    set(CMAKE_CXX_FLAGS_DEBUG "${_opt_level_DEBUG} ${_opt_ext_DEBUG}"
        CACHE STRING "Flags used by the compiler during Debug builds."
        FORCE)
    set(CMAKE_C_FLAGS_DEBUG "${_opt_level_DEBUG} ${_opt_ext_DEBUG}"
        CACHE STRING "Flags used by the compiler during Debug builds."
        FORCE)
    set(CMAKE_Fortran_FLAGS_DEBUG "${_opt_level_DEBUG} ${_opt_ext_DEBUG}"
        CACHE STRING "Flags used by the compiler during Debug builds."
        FORCE)

    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${_opt_level_RELWITHDEBINFO} ${_opt_ext_RELWITHDEBINFO}"
        CACHE STRING "Flags used by the compiler during Release with Debug Info builds."
        FORCE)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${_opt_level_RELWITHDEBINFO} ${_opt_ext_RELWITHDEBINFO}"
        CACHE STRING "Flags used by the compiler during Release with Debug Info builds."
        FORCE)
    set(CMAKE_Fortran_FLAGS_RELWITHDEBINFO "${_opt_level_RELWITHDEBINFO} ${_opt_ext_RELWITHDEBINFO}"
        CACHE STRING "Flags used by the compiler during Release with Debug Info builds."
        FORCE)

    set(CMAKE_CXX_FLAGS_COVERAGE "--coverage"
        CACHE STRING "Flags used by the compiler during coverage builds."
        FORCE)
    set(CMAKE_C_FLAGS_COVERAGE "--coverage"
        CACHE STRING "Flags used by the compiler during coverage builds."
        FORCE)
    set(CMAKE_Fortran_FLAGS_COVERAGE "--coverage"
        CACHE STRING "Flags used by the compiler during coverage builds."
        FORCE)

    set(CMAKE_CXX_FLAGS_PROFILE "-pg"
        CACHE STRING "Flags used by the compiler during profile builds."
        FORCE)
    set(CMAKE_C_FLAGS_PROFILE "-pg"
        CACHE STRING "Flags used by the compiler during profile builds."
        FORCE)
    set(CMAKE_Fortran_FLAGS_PROFILE "-pg"
        CACHE STRING "Flags used by the compiler during profile builds."
        FORCE)


    # The others are already marked as 'advanced' by CMake, these are custom.
    mark_as_advanced(CMAKE_C_FLAGS_COVERAGE CMAKE_CXX_FLAGS_COVERAGE CMAKE_Fortran_FLAGS_COVERAGE
                     CMAKE_C_FLAGS_PROFILE CMAKE_CXX_FLAGS_PROFILE CMAKE_Fortran_FLAGS_PROFILE)
    mark_as_advanced(CMAKE_Fortran_COMPILER CMAKE_Fortran_FLAGS
                     CMAKE_Fortran_FLAGS_RELEASE CMAKE_Fortran_FLAGS_RELWITHDEBINFO)

  endif()

  #--- Link shared flags -------------------------------------------------------
  if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000"
        CACHE STRING "Flags used by the linker during the creation of dll's."
        FORCE)
    set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000"
        CACHE STRING "Flags used by the linker during the creation of modules."
        FORCE)
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000"
        CACHE STRING "Flags used by the linker during the creation of executables."
        FORCE)
  endif()

  if(APPLE)
    # special link options for MacOSX
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup"
        CACHE STRING "Flags used by the linker during the creation of dll's."
        FORCE)
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -undefined dynamic_lookup"
        CACHE STRING "Flags used by the linker during the creation of modules."
        FORCE)
  endif()

  # prevent resetting of the flags
  set(GAUDI_FLAGS_SET "${GAUDI_FLAGS_OPTIONS}"
      CACHE INTERNAL "flag to check if the compilation flags have already been set"
      FORCE)
endif()


if(UNIX)
  add_definitions(-D_GNU_SOURCE -Dunix -Df2cFortran)

  if (CMAKE_SYSTEM_NAME MATCHES Linux)
    add_definitions(-Dlinux)
  endif()
endif()

if(MSVC90)
  add_definitions(/wd4275 /wd4251 /wd4351)
  add_definitions(-DBOOST_ALL_DYN_LINK -DBOOST_ALL_NO_LIB)
  add_definitions(/nologo)
endif()

if(APPLE)
  # by default, CMake uses the option -bundle for modules, but we need -dynamiclib for them too
  string(REPLACE "-bundle" "-dynamiclib" CMAKE_SHARED_MODULE_CREATE_C_FLAGS "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS}")
  string(REPLACE "-bundle" "-dynamiclib" CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS}")
endif()

#--- Special build flags -------------------------------------------------------
if ((GAUDI_V21 OR G21_HIDE_SYMBOLS) AND (BINARY_TAG_COMP_NAME STREQUAL "gcc" AND
                                         NOT BINARY_TAG_COMP_VERSION VERSION_LESS "4.0"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif()

# handle options to choose the  version of the C++ standard
string(TOLOWER "${GAUDI_CXX_STANDARD}" GAUDI_CXX_STANDARD)
if(GAUDI_CXX_STANDARD STREQUAL "ansi")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=${GAUDI_CXX_STANDARD}")
  if(NOT GAUDI_CXX_STANDARD STREQUAL "c++98")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
  endif()
endif()

if(BINARY_TAG_COMP_NAME STREQUAL "clang" AND BINARY_TAG_COMP_VERSION VERSION_EQUAL "3.7")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --gcc-toolchain=${lcg_system_compiler_path}")
endif()

if(NOT GAUDI_V21)
  if(GAUDI_V22)
    add_definitions(-DGAUDI_V22_API)
  else()
    add_definitions(-DGAUDI_V20_COMPAT)
  endif()
  # special case
  if(G21_HIDE_SYMBOLS AND (BINARY_TAG_COMP_NAME STREQUAL "gcc" AND
                           NOT BINARY_TAG_COMP_VERSION VERSION_LESS "4.0"))
    add_definitions(-DG21_HIDE_SYMBOLS)
  endif()
  #
  foreach (feature G21_NEW_INTERFACES G21_NO_DEPRECATED G22_NEW_SVCLOCATOR)
    if (${feature})
      add_definitions(-D${feature})
    endif()
  endforeach()
endif()

#--- Tuning of warnings --------------------------------------------------------
if(GAUDI_HIDE_WARNINGS)
  if(BINARY_TAG_COMP_NAME STREQUAL "clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -Wno-deprecated -Wno-overloaded-virtual -Wno-char-subscripts -Wno-unused-parameter -Wno-unused-local-typedefs -Wno-missing-braces")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-empty-body")
    if(BINARY_TAG_COMP_VERSION VERSION_GREATER "4.7")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    endif()
  endif()
endif()

if(GAUDI_DIAGNOSTICS_COLOR)
  foreach(_language CXX C Fortran)
    set(CMAKE_${_language}_FLAGS "${CMAKE_${_language}_FLAGS} -fdiagnostics-color")
  endforeach()
endif()

#--- Special flags -------------------------------------------------------------
# FIXME: enforce the use of Boost Filesystem V3 to be compatible between 1.44 and 1.48
add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
# FIXME: enforce the use of Boost Phoenix V3 (V2 does not work with recent compilers)
#        see http://stackoverflow.com/q/20721486
#        and http://stackoverflow.com/a/20440238/504346
add_definitions(-DBOOST_SPIRIT_USE_PHOENIX_V3)

if(GAUDI_ATLAS)
  # FIXME: this macro is used in ATLAS to simplify the migration to Gaudi v25,
  #        unfortunately it's not possible to detect the version of Gaudi at this point
  #        so we assume that any CMake-based build in ATLAS uses Gaudi >= v25
  add_definitions(-DHAVE_GAUDI_PLUGINSVC)

  add_definitions(-DATLAS_GAUDI_V21)
  add_definitions(-DATLAS)
  include(AthenaBuildFlags OPTIONAL)
else()
  # FIXME: these macros are LHCb specific, but we do not have yet a way to set
  # compile flags in a project, such that they are inherited by other projects.
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DGOD_NOALLOC)
  endif()
endif()
