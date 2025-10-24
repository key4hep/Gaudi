#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
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
                  SOURCES src/Lib/TESSerializer.cpp
                  LINK
                     PUBLIC GaudiKernel
                            Python::Python
                            ROOT::Net
                            ROOT::RIO
                            ROOT::Thread)

# Build the plugin
gaudi_add_module(GaudiMP
                 SOURCES
                     src/component/IoComponentMgr.cpp
                     src/component/RecordOutputStream.cpp
                     src/component/ReplayOutputStream.cpp
                 LINK GaudiMPLib
                      Python::Python
                      ROOT::Net
                      ROOT::RIO
                      ROOT::Thread)

# ROOT dictionaries
gaudi_add_dictionary(GaudiMPDict
                     HEADERFILES dict/gaudimp_dict.h
                     SELECTION dict/selection.xml
                     LINK GaudiMPLib)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)
