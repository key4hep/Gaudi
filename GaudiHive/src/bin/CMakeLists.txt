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
gaudi_subdir(bin)

find_package(Boost REQUIRED COMPONENTS program_options)

gaudi_depends_on_subdirs(GaudiKernel)

# Hide some Boost/ROOT compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#---Util Executables--------------------------------------------------------
#set(CMAKE_BUILD_TYPE Release)
gaudi_add_executable(makeThesis makeThesis.cpp )
gaudi_add_executable(concurrentRun concurrentRun.cpp
                     LINK_LIBRARIES GaudiKernel ${Boost_LIBRARIES}
                    )
