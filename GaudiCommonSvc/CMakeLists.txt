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
# GaudiCommonSvc subdirectory

# Build the library
if(GAUDI_USE_AIDA)
  gaudi_add_library(GaudiCommonSvcLib
                    SOURCES src/HistogramSvc/HistogramSvc.cpp
                            src/HistogramSvc/AIDA_visibility_hack.cpp
                            src/HistogramSvc/H1D.cpp
                            src/HistogramSvc/H2D.cpp
                            src/HistogramSvc/H3D.cpp
                            src/HistogramSvc/P1D.cpp
                            src/HistogramSvc/P2D.cpp
                    LINK
                      PUBLIC GaudiKernel
                             Boost::headers
                             Boost::filesystem
                             Rangev3::rangev3
                             ROOT::Hist
                             ROOT::RIO
                             AIDA::aida)
endif()

# Build the plugin
gaudi_add_module(GaudiCommonSvc
                 SOURCES src/AlgContextSvc.cpp
                         src/AuditorSvc.cpp
                         src/ChronoStatSvc.cpp
                         src/DataSvc/EvtDataSvc.cpp
                         src/DataSvc/EvtStoreSvc.cpp
                         src/DataSvc/FileRecordDataSvc.cpp
                         src/DataSvc/MultiStoreSvc.cpp
                         src/DataSvc/PartitionSwitchAlg.cpp
                         src/DataSvc/PartitionSwitchTool.cpp
                         src/DataSvc/RecordDataSvc.cpp
                         src/DataSvc/RunRecordDataSvc.cpp
                         src/DataSvc/StoreExplorerAlg.cpp
                         src/DataSvc/StoreSnifferAlg.cpp
                         src/HistogramPersistencySvc/RootHistogramSink.cpp
                         src/PersistencySvc/CopyInputStream.cpp
                         src/PersistencySvc/DataSvcFileEntriesTool.cpp
                         src/PersistencySvc/DetPersistencySvc.cpp
                         src/PersistencySvc/EvtCollectionStream.cpp
                         src/PersistencySvc/EvtPersistencySvc.cpp
                         src/PersistencySvc/InputCopyStream.cpp
                         src/PersistencySvc/OutputStream.cpp
                         src/PersistencySvc/PersistencySvc.cpp
                         src/PersistencySvc/RecordStream.cpp
                         src/PersistencySvc/RunRecordStream.cpp
                         src/PersistencySvc/SequentialOutputStream.cpp
                         src/PersistencySvc/TagCollectionStream.cpp
                 LINK GaudiKernel
                      Boost::headers
                      Boost::filesystem
                      Rangev3::rangev3
                      ROOT::Hist
                      ROOT::RIO
                      ${GAUDI_ATOMIC_LIBS})
if(GAUDI_USE_AIDA)
  target_sources(GaudiCommonSvc PRIVATE src/HistogramSvc/Factory.cpp
                                        src/HistogramPersistencySvc/HistogramPersistencySvc.cpp)
  target_link_libraries(GaudiCommonSvc PRIVATE AIDA::aida GaudiCommonSvcLib)
endif()

if(BUILD_TESTING)
    gaudi_add_executable(test_AlgContextSvc
        SOURCES
            src/AlgContextSvc.cpp
        LINK
            Boost::headers
            Catch2::Catch2WithMain
            Gaudi::GaudiKernel
        TEST
    )
    target_compile_definitions(test_AlgContextSvc PRIVATE BUILD_UNIT_TESTS)

    gaudi_add_executable(GaudiCommonSvc_tests
        SOURCES
            tests/src/test_PersistencySvc.cpp
            src/PersistencySvc/PersistencySvc.cpp
        LINK
            Boost::headers
            Catch2::Catch2WithMain
            Gaudi::GaudiKernel
        TEST
    )
endif()
