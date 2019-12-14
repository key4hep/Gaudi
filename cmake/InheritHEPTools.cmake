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
# Special toolchain file that looks for the projects used and choose the version
# of the heptools toolchain from them.

cmake_minimum_required(VERSION 3.6)

# FIXME: this is for backward compatibility with the olf LHCb minimal toolchain
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_MODULE_PATH})
list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)

# detect the required heptols version
function(guess_heptools_version var)

  if(NOT projects)
    include(GaudiToolchainMacros)
    init()
    find_projects(projects tools ${CMAKE_SOURCE_DIR}/CMakeLists.txt)
  endif()

  compatible_binary_tags(_tags)
  foreach(_project ${projects})
    foreach(_tag ${_tags})
      file(GLOB config ${${_project}_ROOT_DIR}/InstallArea/${_tag}/*Config.cmake)
      if(config)
        file(READ ${config} config_file)
        if(config_file MATCHES "set *\\( *[^ ]*_heptools_version *([^ ]*) *\\)")
          set(${var} ${CMAKE_MATCH_1} PARENT_SCOPE)
          message(STATUS "Detected heptools version ${CMAKE_MATCH_1} in ${${_project}_ROOT_DIR} from ${config}")
          return()
        endif()
      endif()
    endforeach()
  endforeach()

endfunction()

include(UseHEPTools)

macro(inherit_heptools)
  # FIXME: this is for backward compatibility with the olf LHCb minimal toolchain
  if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeTmp")
    guess_heptools_version(heptools_version)
    if(heptools_version)
      use_heptools(${heptools_version})
    endif()
  endif()
endmacro()
