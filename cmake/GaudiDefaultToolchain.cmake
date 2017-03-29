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
if(NOT LCG_SYSTEM)
  set(LCG_SYSTEM ${BINARY_TAG_ARCH}-${BINARY_TAG_OS}-${BINARY_TAG_COMP}
      CACHE STRING "Platform id of the target system or a compatible one.")
  mark_as_advanced(LCG_SYSTEM LCG_platform LCG_system)
endif()
set(LCG_ARCH "${BINARY_TAG_ARCH}")
set(LCG_HOST_ARCH "${HOST_BINARY_TAG_ARCH}")
set(LCG_COMP "${BINARY_TAG_COMP_NAME}")
string(REPLACE "." "" LCG_COMPVERS "${BINARY_TAG_COMP_VERSION}")
set(LCG_BUILD_TYPE ${BINARY_TAG_TYPE})
set(LCG_platform ${LCG_SYSTEM}-${BINARY_TAG_TYPE})
set(LCG_system   ${LCG_SYSTEM}-opt)
