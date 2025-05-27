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
# GaudiSvc subdirectory

# Build the plugin
gaudi_add_module(GaudiSvc
                 SOURCES src/CPUCrunchSvc/CPUCrunchSvc.cpp
                         src/DetectorDataSvc/DetDataSvc.cpp
                         src/FileMgr/FileMgr.cpp
                         src/FileMgr/POSIXFileHandler.cpp
                         src/FileMgr/RootFileHandler.cpp
                         src/MetaDataSvc/MetaDataSvc.cpp
                         src/NTupleSvc/CollectionCloneAlg.cpp
                         src/NTupleSvc/NTupleSvc.cpp
                         src/NTupleSvc/TagCollectionSvc.cpp
                         src/THistSvc/THistSvc.cpp
                   LINK GaudiKernel
                        Boost::headers
                        Boost::regex
                        ROOT::Hist
                        ROOT::RIO
                        ROOT::Tree
                        ROOT::Net
                        ROOT::Matrix
                        ROOT::Thread
                        ROOT::MathCore)
# ATLAS may want to append GENCONF_OPTIONS --user-module=GaudiSvc.ExtraModules

if(GAUDI_USE_CLHEP)
   target_sources(GaudiSvc PRIVATE src/RndmGenSvc/HepRndmEngines.cpp
                                   src/RndmGenSvc/RndmEngine.cpp
                                   src/RndmGenSvc/RndmGenSvc.cpp
                                   src/RndmGenSvc/HepRndmGenerators.cpp
                                   src/RndmGenSvc/RndmGen.cpp)
   target_link_libraries(GaudiSvc PRIVATE CLHEP::CLHEP)
endif()

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)
