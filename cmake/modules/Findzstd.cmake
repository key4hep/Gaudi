#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# - Locate the zstd compression library
# Defines:
#
#  zstd_FOUND
#  zstd_INCLUDE_DIR
#  zstd_INCLUDE_DIRS (not cached)
#  zstd_LIBRARY
#  zstd_LIBRARIES (not cached)
#  zstd_VERSION
#
# Imports:
#
#  zstd::libzstd
#
# Usage of the target instead of the variables is advised
#
# Not using pkg-config on purpose: zstd usually comes from the system while the rest
# of the stack does not, and the pkg-config binaries shipped in LCG views only look
# at PKG_CONFIG_PATH, which those views point at themselves.

# Find quietly if already found before
if(DEFINED CACHE{zstd_INCLUDE_DIR})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

find_path(zstd_INCLUDE_DIR zstd.h
          HINTS $ENV{ZSTD_ROOT_DIR}/include ${ZSTD_ROOT_DIR}/include)
find_library(zstd_LIBRARY NAMES zstd
             HINTS $ENV{ZSTD_ROOT_DIR}/lib ${ZSTD_ROOT_DIR}/lib)

set(zstd_INCLUDE_DIRS ${zstd_INCLUDE_DIR})
set(zstd_LIBRARIES ${zstd_LIBRARY})

if(zstd_INCLUDE_DIR AND EXISTS "${zstd_INCLUDE_DIR}/zstd.h")
  file(STRINGS "${zstd_INCLUDE_DIR}/zstd.h" _zstd_version_lines
       REGEX "^#define +ZSTD_VERSION_(MAJOR|MINOR|RELEASE) +[0-9]+")
  string(REGEX REPLACE ".*ZSTD_VERSION_MAJOR +([0-9]+).*" "\\1" _zstd_major "${_zstd_version_lines}")
  string(REGEX REPLACE ".*ZSTD_VERSION_MINOR +([0-9]+).*" "\\1" _zstd_minor "${_zstd_version_lines}")
  string(REGEX REPLACE ".*ZSTD_VERSION_RELEASE +([0-9]+).*" "\\1" _zstd_release "${_zstd_version_lines}")
  set(zstd_VERSION "${_zstd_major}.${_zstd_minor}.${_zstd_release}")
  unset(_zstd_version_lines)
  unset(_zstd_major)
  unset(_zstd_minor)
  unset(_zstd_release)
endif()

# handle the QUIETLY and REQUIRED arguments and set zstd_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(zstd
                                 REQUIRED_VARS zstd_INCLUDE_DIR zstd_LIBRARY
                                 VERSION_VAR zstd_VERSION)

mark_as_advanced(zstd_FOUND zstd_INCLUDE_DIR zstd_LIBRARY)

# Modernisation: create an interface target to link against
if(TARGET zstd::libzstd)
  return()
endif()
if(zstd_FOUND)
  add_library(zstd::libzstd IMPORTED INTERFACE)
  target_include_directories(zstd::libzstd SYSTEM INTERFACE "${zstd_INCLUDE_DIR}")
  target_link_libraries(zstd::libzstd INTERFACE "${zstd_LIBRARY}")
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: zstd::libzstd")
  endif()
endif()
