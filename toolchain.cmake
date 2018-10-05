# Special wrapper to load the declared version of the heptools toolchain.
set(heptools_version 94)

cmake_minimum_required(VERSION 2.8.5)
if($ENV{HEPTOOLS_VERSION})
  set(heptools_version $ENV{HEPTOOLS_VERSION})
endif()

# this check is needed because the toolchain is called when checking the
# compiler (without the proper cache)
if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeTmp")

 # Note: normally we should look for GaudiDefaultToolchain.cmake, but in Gaudi
 # it is not needed
 include(${CMAKE_SOURCE_DIR}/cmake/GaudiDefaultToolchain.cmake)

 # FIXME: make sure we do not pick up ninja from LCG (it requires LD_LIBRARY_PATH set)
 if(CMAKE_PREFIX_PATH AND CMAKE_VERSION VERSION_GREATER "3.6.0")
  list(FILTER CMAKE_PREFIX_PATH EXCLUDE REGEX "(LCG_|lcg/nightlies).*ninja")
 endif()
endif()
