#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Special wrapper to load the declared version of the heptools toolchain.
set(heptools_version 96b)

cmake_minimum_required(VERSION 3.6)
if(NOT "$ENV{HEPTOOLS_VERSION}" STREQUAL "")
  set(heptools_version $ENV{HEPTOOLS_VERSION})
endif()

# this check is needed because the toolchain is called when checking the
# compiler (without the proper cache)
if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeTmp")

  # Note: normally we should look for GaudiDefaultToolchain.cmake, but in Gaudi
  # it is not needed
  include(${CMAKE_SOURCE_DIR}/cmake/GaudiDefaultToolchain.cmake)

  # FIXME: make sure we do not pick up unwanted/problematic projects from LCG
  if(CMAKE_PREFIX_PATH)
    # - ninja (it requires LD_LIBRARY_PATH set to run)
    # - Gaudi (we do not want to use it from LCG)
    # - xenv (conflicts with the version in the build environment)
    list(FILTER CMAKE_PREFIX_PATH EXCLUDE REGEX "(LCG_|lcg/nightlies).*(ninja|Gaudi|xenv)")
  endif()
endif()
