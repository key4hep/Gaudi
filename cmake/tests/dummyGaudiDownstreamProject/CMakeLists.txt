#####################################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
cmake_minimum_required(VERSION 3.15)

project(dummyGaudiDownstreamProject
        VERSION 0.1
        LANGUAGES CXX
        DESCRIPTION "Project to test Gaudi as a dependency")

if(NOT TARGET Gaudi::GaudiPluginService)
    find_package(Gaudi REQUIRED) # Imports all the targets and functions and find the dependencies
endif()

# Set the C++ standard
set(CMAKE_CXX_STANDARD ${GAUDI_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

gaudi_add_executable(mainexe SOURCES src/main.cpp LINK Gaudi::GaudiPluginService)
gaudi_generate_version_header_file() # generate DUMMYGAUDIDOWNSTREAMPROJECT_VERSION.h
gaudi_generate_version_header_file(Main) # generate MainVersion.h
target_include_directories(mainexe PRIVATE ${CMAKE_BINARY_DIR}/include)


###################### Simple unit tests beyond this line ######################

# Check validity of GAUDI_CXX_STANDARD
if(NOT GAUDI_CXX_STANDARD)
    message(FATAL_ERROR "GAUDI_CXX_STANDARD is not set or empty.")
endif()

# Check that we have a run target
if(NOT TARGET run)
    message(FATAL_ERROR "There is no run target.")
endif()

# Targets needed for gaudi_add_module()
foreach(target IN ITEMS Gaudi::listcomponents Gaudi::genconf)
    # Check they are targets
    if(NOT TARGET ${target})
        message(FATAL_ERROR "${target} is not a target.")
    endif()
    # Check they exist on the disk if not build by the project
    get_target_property(target_imported ${target} IMPORTED)
    if(target_imported)
        get_target_property(loc ${target} LOCATION)
        if(NOT EXISTS "${loc}")
            message(FATAL_ERROR "${target} is imported from ${loc} but the file does not exist.")
        endif()
    endif()
endforeach()
