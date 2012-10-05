# Special wrapper to load the declared version of the heptools toolchain.
set(heptools_version 64a)

find_file(toolchain_file
          NAMES heptools-${heptools_version}.cmake
          HINTS ENV CMTPROJECTPATH
          PATHS ${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain
          PATH_SUFFIXES toolchain)

if(NOT toolchain_file)
  message(FATAL_ERROR "Cannot find heptools-${heptools_version}.cmake.")
endif()

include(${toolchain_file})
