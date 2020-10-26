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
# RootCnv subdirectory

# Build the library
gaudi_add_library(RootCnvLib
                  SOURCES src/RootCnvSvc.cpp
                          src/RootDataConnection.cpp
                          src/RootEvtSelector.cpp
                          src/RootNTupleCnv.cpp
                          src/RootStatCnv.cpp
                          src/RootDatabaseCnv.cpp
                          src/RootDirectoryCnv.cpp
                          src/RootIOHandler.cpp
                          src/RootPerfMonSvc.cpp
                          src/SysProcStat.cpp
                  LINK
                    PUBLIC GaudiKernel
                           GaudiUtilsLib
                           ROOT::RIO
                           ROOT::Tree
                           ROOT::TreePlayer
                           ROOT::MathCore
                           ROOT::Net
                           ROOT::Graf3d
                           ROOT::Graf
                           ROOT::Gpad)

# Build the plugin
gaudi_add_module(RootCnv
                 SOURCES components/Components.cpp
                 LINK RootCnvLib)

# Build executable
gaudi_add_executable(gaudi_merge
                     SOURCES merge/merge.cpp
                     LINK RootCnvLib)
gaudi_add_executable(extract_event
                     SOURCES merge/extractEvt.cpp
                     LINK RootCnvLib)

# ROOT dictionaries
gaudi_add_dictionary(RootCnvDict
                     HEADERFILES dict/RootCnv_dict.h
                     SELECTION dict/RootCnv_dict.xml
                     LINK RootCnvLib)

set_property(TARGET RootCnvLib RootCnv RootCnvDict
             APPEND PROPERTY COMPILE_DEFINITIONS __POOL_COMPATIBILITY)
        
# Install other scripts
gaudi_install(SCRIPTS)
