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
# GaudiMP subdirectory

# Build the library
gaudi_add_library(GaudiMPLib
                  SOURCES src/Lib/PyROOTPickle.cpp
                          src/Lib/TESSerializer.cpp
                  LINK
                     PUBLIC GaudiKernel
                            Python::Python
                            ROOT::Net
                            ROOT::RIO
                            ROOT::Thread)

# Build the plugin
gaudi_add_module(GaudiMP
                 SOURCES src/component/IoComponentMgr.cpp
                 LINK GaudiMPLib
                      GaudiAlgLib
                      Python::Python
                      ROOT::Net
                      ROOT::RIO
                      ROOT::Thread)
if(GAUDI_USE_AIDA)
   target_sources(GaudiMP PRIVATE src/component/RecordOutputStream.cpp
                                  src/component/ReplayOutputStream.cpp)
   target_link_libraries(GaudiMP PRIVATE AIDA::aida)
endif()

# ROOT dictionaries
gaudi_add_dictionary(GaudiMPDict
                     HEADERFILES dict/gaudimp_dict.h
                     SELECTION dict/selection.xml
                     LINK GaudiMPLib)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)
