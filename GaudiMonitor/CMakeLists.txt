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
# GaudiMonitor subdirectory

# Build the plugin
gaudi_add_module(GaudiMonitor
                 SOURCES src/ExceptionSvc.cpp
                         src/HistorySvc.cpp
                 LINK GaudiKernel)

if(WIN32)
  target_link_libraries(GaudiMonitor PRIVATE ws2_32.lib)
  # These are needed because Windows compiler doesn't like standard C and POSIX functions,
  # but this package contains some imported (external) C file that is compiled here in C++.
  target_compile_definitions(GaudiMonitor PRIVATE _CRT_SECURE_NO_WARNINGS
                                                  _SCL_SECURE_NO_WARNINGS)
  target_compile_options(GaudiMonitor /wd4996)
endif()
