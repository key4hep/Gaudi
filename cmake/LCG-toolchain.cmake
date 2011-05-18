# This toolchain requires the environment variables CMTCONFIG or CMAKECONFIG
if(DEFINED ENV{CMAKECONFIG})
  set(BINARY_TAG $ENV{CMAKECONFIG})
elseif(DEFINED ENV{CMTCONFIG})
  set(BINARY_TAG $ENV{CMTCONFIG})
else()
  message(FATAL_ERROR "Neither CMTCONFIG nor CMAKECONFIG is defined.")
endif()

# Split the binary tag
string(REGEX MATCHALL "[^-]+" out ${BINARY_TAG})
list(GET out 0 arch)
list(GET out 1 os)
list(GET out 2 comp)
list(GET out 3 type)

# architecture
SET(CMAKE_SYSTEM_PROCESSOR ${arch})

# system name
if(os STREQUAL "winxp")
  SET(CMAKE_SYSTEM_NAME Windows)
elseif(os MATCHES "mac.*")
  SET(CMAKE_SYSTEM_NAME Darwin)
elseif(os MATCHES "slc[456]")
  SET(CMAKE_SYSTEM_NAME Linux)
else()
  message(FATAL_ERROR "OS ${os} is not supported.")
endif()

# compiler (from a list of supported ones)
if(comp STREQUAL "gcc43")
  SET(CMAKE_C_COMPILER   lcg-gcc-4.3.2)
  SET(CMAKE_CXX_COMPILER lcg-g++-4.3.2)
elseif(comp STREQUAL "icc11")
  SET(CMAKE_C_COMPILER   icc)
  SET(CMAKE_CXX_COMPILER icpc)
else()
  message(FATAL_ERROR "Uknown compiler ${comp}.")
endif()

# build type
if(type STREQUAL "opt")
  SET(type Release)
elseif(type STREQUAL "dbg")
  SET(type Debug)
else()
  message(FATAL_ERROR "Build flavour ${type} not supported.")
endif()
SET(CMAKE_BUILD_TYPE ${type} CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)

SET(LCG_platform ${BINARY_TAG})
SET(LCG_system ${arch}-${os}-${comp}-opt)
