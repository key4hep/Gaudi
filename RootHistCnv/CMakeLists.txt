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
# RootHistCnv subdirectory

# If AIDA is missing, don't build this module:
if(NOT GAUDI_USE_AIDA)
   message(AUTHOR_WARNING "RootHistCnv is only available if AIDA is enabled")
   return()
endif()

# Build the plugin
gaudi_add_module(RootHistCnv
                 SOURCES src/DirectoryCnv.cpp
                         src/PersSvc.cpp
                         src/RConverter.cpp
                         src/RCWNTupleCnv.cpp
                         src/RDirectoryCnv.cpp
                         src/RFileCnv.cpp
                         src/RHistogramCnv.cpp
                         src/RNTupleCnv.cpp
                         src/RootCompressionSettings.cpp
                         src/RootDirFcn.cpp
                         src/RRWNTupleCnv.cpp
                 LINK GaudiKernel
                      ROOT::RIO
                      ROOT::Hist
                      ROOT::Matrix
                      ROOT::Tree
                      ROOT::Thread
                      ROOT::MathCore
                      ROOT::Net
                      Boost::headers
                      AIDA::aida)
