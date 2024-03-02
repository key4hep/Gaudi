#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Gaudi subdirectory

if(NOT GAUDI_ENABLE_GAUDIALG)
   return()
endif()

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
                          src/lib/Tuple.cpp
                          src/lib/TupleObj.cpp
                          src/lib/AlgDecorators.cpp
                  LINK
                    PUBLIC
                      Gaudi::GaudiUtilsLib
                      Gaudi::GaudiPythonLib
                      Gaudi::Functional
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
                                      src/lib/GaudiTupleTool.cpp
                                      src/lib/HistoDecorator.cpp)
   target_link_libraries(GaudiAlgLib PUBLIC AIDA::aida)
   if(GAUDI_USE_CLHEP)
      target_sources(GaudiAlgLib PRIVATE src/lib/TupleDecorator.cpp)
      target_link_libraries(GaudiAlgLib PUBLIC CLHEP::CLHEP)
   endif()
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

gaudi_add_dictionary(GaudiAlgDict
                     HEADERFILES dict/dictionary.h
                     SELECTION dict/selection.xml
                     LINK GaudiAlgLib)

if(GAUDI_USE_AIDA)
    target_compile_definitions(GaudiAlgDict PRIVATE AIDA_FOUND)
endif()
if(GAUDI_USE_CLHEP)
    target_compile_definitions(GaudiAlgDict PRIVATE CLHEP_FOUND)
endif()

gaudi_install(PYTHON)

if(BUILD_TESTING)
   # Ignore examples if not building tests
   # see https://gitlab.cern.ch/atlas/atlasexternals/-/merge_requests/664#note_3395313
   gaudi_add_module(GaudiAlgExamples
      SOURCES
         src/examples/GaudiCommonTests.cpp
         src/examples/MyGaudiAlgorithm.cpp
         src/examples/MyGaudiTool.cpp
         src/examples/SelCreate.cpp
         src/examples/SelFilter.cpp
      LINK
         GaudiAlgLib
         Gaudi::GaudiTestSuiteLib
   )

   if(GAUDI_USE_AIDA)
      target_sources(GaudiAlgExamples PRIVATE
         src/examples/Aida2Root.cpp
         src/examples/GaudiHistoAlgorithm.cpp
         src/examples/HistoTimingAlg.cpp
         src/examples/EvtColAlg.cpp
         src/examples/ExtendedEvtCol.cpp
      )
      target_link_libraries(GaudiAlgExamples PRIVATE AIDA::aida)
      if(GAUDI_USE_CLHEP)
         target_sources(GaudiAlgExamples PRIVATE
            src/examples/TupleAlg2.cpp
            src/examples/TupleAlg3.cpp
            src/examples/TupleAlg.cpp
            src/examples/TupleDef.cpp
         )
         target_link_libraries(GaudiAlgExamples PRIVATE CLHEP::CLHEP)
      endif()
   endif()
endif()

gaudi_add_tests(QMTest)
