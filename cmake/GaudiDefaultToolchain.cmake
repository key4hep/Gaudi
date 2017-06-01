set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_MODULE_PATH})

include(GaudiToolchainMacros)

init()
find_projects(projects tools ${CMAKE_SOURCE_DIR}/CMakeLists.txt)

if(heptools_version)
  include(UseHEPTools)
  use_heptools(${heptools_version})
else()
  include(InheritHEPTools)
  inherit_heptools()
endif()

set_paths_from_projects(${tools} ${projects})

# set legacy variables for backward compatibility
find_package(GaudiProject QUIET)
if(NOT EXISTS "${GaudiProject_DIR}/BinaryTagUtils.cmake")
  # with newer versions of Gaudi these variables are set after the toolchain,
  # but we have to set them here for old versions

  # if we actually found an LCG info file, we use the informations from there
  if(LCG_TOOLCHAIN_INFO)
    string(REGEX MATCH ".*/LCG_externals_(.+)\\.txt" out "${LCG_TOOLCHAIN_INFO}")
    set(LCG_platform ${CMAKE_MATCH_1})

    # set LCG_ARCH, LCG_COMP and LCG_TYPE
    parse_binary_tag(LCG "${CMAKE_MATCH_1}")

    set(LCG_HOST_ARCH "${HOST_BINARY_TAG_ARCH}")
    set(LCG_SYSTEM ${LCG_ARCH}-${LCG_OS}-${LCG_COMP})
    set(LCG_system ${LCG_SYSTEM}-opt)
    set(LCG_BUILD_TYPE ${LCG_TYPE})

    # match old-style LCG_COMP value
    set(LCG_COMP "${BINARY_TAG_COMP_NAME}")
    string(REPLACE "." "" LCG_COMPVERS "${BINARY_TAG_COMP_VERSION}")

    # Convert LCG_BUILD_TYPE to CMAKE_BUILD_TYPE
    if(LCG_BUILD_TYPE STREQUAL "opt")
      set(type Release)
    elseif(LCG_BUILD_TYPE STREQUAL "dbg")
      set(type Debug)
    elseif(LCG_BUILD_TYPE STREQUAL "cov")
      set(type Coverage)
    elseif(LCG_BUILD_TYPE STREQUAL "pro")
      set(type Profile)
    else()
      message(FATAL_ERROR "LCG build type ${type} not supported.")
    endif()
    set(CMAKE_BUILD_TYPE ${type} CACHE STRING
        "Choose the type of build, options are: empty, Debug, Release, Coverage, Profile, RelWithDebInfo, MinSizeRel.")
  endif()
elseif(EXISTS "${GaudiProject_DIR}/../GaudiConfig.cmake")
  # Special workaround for Gaudi v28r2
  file(READ "${GaudiProject_DIR}/../GaudiConfig.cmake" out)
  if(out MATCHES "Gaudi_VERSION v28r2")
    string(REGEX MATCH ".*/LCG_externals_(.+)\\.txt" out "${LCG_TOOLCHAIN_INFO}")
    set(LCG_platform ${CMAKE_MATCH_1}
        CACHE STRING "Platform ID for the AA project binaries.")
  endif()
endif()
