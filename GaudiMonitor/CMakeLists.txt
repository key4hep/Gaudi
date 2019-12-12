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
gaudi_subdir(GaudiMonitor)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost REQUIRED)

# Hide some Boost compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

if(WIN32)
  set(winsocklib ws2_32.lib)
endif()
#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiMonitor src/*.cpp
                 LINK_LIBRARIES GaudiKernel ${winsocklib})

if(WIN32 AND GAUDI_HIDE_WARNINGS)
  # These are needed because Windows compiler doesn't like standard C and POSIX functions,
  # but this package containes some imported (external) C file that is compiled here in C++.
  set_property(TARGET GaudiMonitor
               PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS;_SCL_SECURE_NO_WARNINGS)
  set_property(TARGET GaudiMonitor
               PROPERTY COMPILE_FLAGS /wd4996)
endif()
