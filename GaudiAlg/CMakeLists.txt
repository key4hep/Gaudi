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
# Gaudi subdirectory

# Build the library
gaudi_add_library(GaudiAlgLib
                  SOURCES src/lib/EventCounter.cpp
                          src/lib/FixTESPath.cpp
                          src/lib/GaudiAlgorithm.cpp
                          src/lib/GaudiCommon.icpp
                          src/lib/GaudiHistoID.cpp
                          src/lib/GaudiSequencer.cpp
                          src/lib/GaudiTool.cpp
                          src/lib/GetAlg.cpp
                          src/lib/Prescaler.cpp
                          src/lib/Sequencer.cpp
                          src/lib/Tuple.cpp
                          src/lib/TupleObj.cpp
                  LINK
                    PUBLIC
                      GaudiUtilsLib
                      Boost::headers
                      Boost::regex
                      Rangev3::rangev3)
if(GAUDI_USE_AIDA)
   target_sources(GaudiAlgLib PRIVATE src/lib/Fill.cpp
                                      src/lib/Print.cpp
                                      src/lib/GaudiHistoAlg.cpp
                                      src/lib/GaudiHistoTool.cpp
                                      src/lib/GetAlgs.cpp
                                      src/lib/GaudiTupleAlg.cpp
                                      src/lib/GaudiTupleTool.cpp)
   target_link_libraries(GaudiAlgLib PUBLIC AIDA::aida)
endif()

# Build the plugin
gaudi_add_module(GaudiAlg
                 SOURCES
                    src/components/ErrorTool.cpp
                    src/components/TimingAuditor.cpp
                    src/components/EventNodeKiller.cpp
                    src/components/GaudiAlg_entries.cpp
                    src/components/TimerForSequencer.cpp
                 LINK
                    GaudiAlgLib
                    GaudiKernel
                    ROOT::Core
                    ROOT::RIO)
if(GAUDI_USE_AIDA)
   target_sources(GaudiAlg PRIVATE src/components/HistoTool.cpp
                                   src/components/TupleTool.cpp
                                   src/components/SequencerTimerTool.cpp)
   target_link_libraries(GaudiAlg PRIVATE AIDA::aida)
endif()
