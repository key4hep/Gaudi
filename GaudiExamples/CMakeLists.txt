#####################################################################################
# (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# GaudiExamples subdirectory
if(NOT BUILD_TESTING)
  # Ignore examples if not building tests
  # see https://gitlab.cern.ch/atlas/atlasexternals/-/merge_requests/664#note_3395313
  return()
endif()

# Build the library
gaudi_add_library(GaudiExamplesLib
                  SOURCES src/Lib/Counter.cpp
                          src/Lib/Event.cpp
                          src/Lib/MyTrack.cpp
                          src/Lib/MyVertex.cpp
                  LINK PUBLIC GaudiKernel)

# Build the plugin
gaudi_add_module(GaudiExamples
                 SOURCES src/AbortEvent/AbortEventAlg.cpp
                         src/AlgSequencer/HelloWorld.cpp
                         src/AlgSequencer/ParentAlg.cpp
                         src/AlgSequencer/StopperAlg.cpp
                         src/AlgSequencer/SubAlg.cpp
                         src/AlgSequencer/TemplatedAlg.cpp
                         src/AlgTools/MyAlgorithm.cpp
                         src/AlgTools/MyGaudiAlgorithm.cpp
                         src/AlgTools/MyGaudiTool.cpp
                         src/AlgTools/MyServiceWithTool.cpp
                         src/AlgTools/MyTool.cpp
                         src/AlgTools/TestTool.cpp
                         src/AlgTools/TestToolAlg.cpp
                         src/AlgTools/TestToolAlgFailure.cpp
                         src/AlgTools/TestToolFailing.cpp
                         src/AnyData/AnyDataAlgorithm.cpp
                         src/AnyData/AnyViewAlgorithm.cpp
                         src/Auditors/AuditorTestAlg.cpp
                         src/Auditors/LoggingAuditor.cpp
                         src/bug_34121/bug34121_MyAlgorithm.cpp
                         src/bug_34121/bug34121_Tool.cpp
                         src/ColorMsg/ColorMsgAlg.cpp
                         src/ConditionAccessor/Backend.cpp
                         src/ConditionAccessor/CondSvc.cpp
                         src/ConditionAccessor/User.cpp
                         src/CounterEx/CounterAlg.cpp
                         src/CounterEx/CounterSvcAlg.cpp
                         src/CounterEx/StatSvcAlg.cpp
                         src/DataOnDemand/DataCreator.cpp
                         src/DataOnDemand/MyDataAlgorithm.cpp
                         src/ExtendedProperties/BoostArrayProperties.cpp
                         src/ExtendedProperties/ExtendedProperties.cpp
                         src/ExtendedProperties/ExtendedProperties2.cpp
                         src/FileMgr/FileMgrTest.cpp
                         src/FunctionalAlgorithms/MakeAndConsume.cpp
                         src/FunctionalAlgorithms/merging_transformer.cpp
                         src/FunctionalAlgorithms/selector.cpp
                         src/FunctionalAlgorithms/TestObjectVersion.cpp
                         src/FunctionalAlgorithms/transformer.cpp
                         src/GaudiCommonTests/GaudiCommonTests.cpp
                         src/Histograms/CounterHistos.cpp
                         src/History/History.cpp
                         src/IncidentSvc/IncidentAsyncTestAlg.cpp
                         src/IncidentSvc/IncidentAsyncTestSvc.cpp
                         src/IncidentSvc/IncidentListenerTest.cpp
                         src/IncidentSvc/IncidentListenerTestAlg.cpp
                         src/IntelProfiler/CpuHungryAlg.cpp
                         src/Maps/MapAlg.cpp
                         src/MultipleLogStreams/QotdAlg.cpp
                         src/NTuples/NTupleAlgorithm.cpp
                         src/PluginService/BackwardCompatibleAliases.cpp
                         src/PluginService/SpecialCustomFactory.cpp
                         src/Properties/CustomPropertiesAlg.cpp
                         src/Properties/PropertyAlg.cpp
                         src/Properties/PropertyProxy.cpp
                         src/QueueingEventProcessor/QueueingEventLoopMgr.cpp
                         src/QueueingEventProcessor/QueueingApplication.cpp
                         src/ReEntAlg/ReEntAlg.cpp
                         src/Selections/SelCreate.cpp
                         src/Selections/SelFilter.cpp
                         src/StringKeys/StringKeyEx.cpp
                         src/SvcInitLoop/LoopAlg.cpp
                         src/SvcInitLoop/ServiceA.cpp
                         src/SvcInitLoop/ServiceB.cpp
                         src/testing/GAUDI-1185.cpp
                         src/testing/HistogramsTests.cpp
                         src/testing/TestingAlgs.cpp
                         src/testing/TestingSvcs.cpp
                         src/testing/HistogramsTests.cpp
                         src/THist/THistRead.cpp
                         src/THist/THistWrite.cpp
                         src/Timing/TimingAlg.cpp
                         src/ToolHandles/Algorithms.cpp
                         src/ToolHandles/FloatTool.cpp
                 LINK GaudiKernel
                      GaudiAlgLib
                      GaudiUtilsLib
                      GaudiExamplesLib
                      ROOT::Tree
                      ROOT::RIO
                      ROOT::Hist
                      ROOT::Net
                      ${GAUDI_ATOMIC_LIBS})

if(NOT ROOT_VERSION VERSION_LESS "6.20")
  find_package(ROOT REQUIRED COMPONENTS ROOTHist)
  target_link_libraries(GaudiExamples PRIVATE ROOT::ROOTHist)
endif()

if(GAUDI_USE_AIDA)
    target_sources(GaudiExamples PRIVATE src/EvtColsEx/EvtColAlg.cpp
                                         src/Histograms/Aida2Root.cpp
                                         src/Histograms/GaudiHistoAlgorithm.cpp
                                         src/Histograms/HistoAlgorithm.cpp
                                         src/Histograms/HistoProps.cpp
                                         src/Histograms/HistoTimingAlg.cpp
                                         src/IO/EvtCollectionSelector.cpp
                                         src/IO/EvtCollectionWrite.cpp
                                         src/IO/EvtExtCollectionSelector.cpp
                                         src/IO/ExtendedEvtCol.cpp
                                         src/IO/ReadAlg.cpp
                                         src/IO/ReadHandleAlg.cpp
                                         src/IO/ReadTES.cpp
                                         src/IO/WriteAlg.cpp
                                         src/IO/WriteHandleAlg.cpp
                                         src/RandomNumber/RandomNumberAlg.cpp)
    target_link_libraries(GaudiExamples PRIVATE AIDA::aida)
endif()
if(GAUDI_USE_HEPPDT)
    target_sources(GaudiExamples PRIVATE src/PartProp/GaudiPPS.cpp
                                         src/PartProp/PartPropExa.cpp)
    target_link_libraries(GaudiExamples PRIVATE HepPDT::heppdt)
endif()
if(GAUDI_USE_CLHEP)
    target_sources(GaudiExamples PRIVATE src/TupleEx/TupleAlg2.cpp
                                         src/TupleEx/TupleAlg3.cpp
                                         src/TupleEx/TupleAlg.cpp
                                         src/TupleEx/TupleDef.cpp)
    target_link_libraries(GaudiExamples PRIVATE CLHEP::CLHEP)
endif()
if(TARGET RootCnvLib)
    target_sources(GaudiExamples PRIVATE src/MultiInput/DumpAddress.cpp
                                         src/MultiInput/MIReadAlg.cpp
                                         src/MultiInput/MIWriteAlg.cpp)
    target_link_libraries(GaudiExamples PRIVATE RootCnvLib)
endif()

# Build the executable
gaudi_add_executable(Allocator
                        SOURCES src/Allocator/Allocator.cpp
                                src/Allocator/MyClass1A.cpp
                                src/Allocator/MyClass1.cpp
                        LINK GaudiExamplesLib
                            GaudiAlgLib
                            GaudiKernel)
gaudi_add_executable(testAppMgrStateMachine
                     SOURCES src/testing/TestAppMgrStateMachine.cpp
                     LINK GaudiKernel)
gaudi_add_executable(QueueingEventProcessorExample
                     SOURCES src/QueueingEventProcessor/main.cpp
                     LINK GaudiKernel)

gaudi_add_executable(ToolVisitorTest_test
                     SOURCES src/ToolVisitorTest/ToolVisitorTest_test.cpp
                             src/ToolVisitorTest/ToolVisitorTestLib.cpp
                     LINK GaudiKernel
                     TEST)

set_target_properties(ToolVisitorTest_test PROPERTIES ENABLE_EXPORTS TRUE)

# Generate GaudiExamples_user.confdb
gaudi_generate_confuserdb()

# Tests
gaudi_add_tests(QMTest)
if(TEST "GaudiExamples.jira.gaudi_1174" AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_property(TEST GaudiExamples.jira.gaudi_1174 PROPERTY DISABLED TRUE)
endif()
foreach(test IN ITEMS
    metadatasvc
    properties.all properties.all2 properties.all_py
    bug_34121_tool_properties_in_python
    deferred_write.enforced deferred_write.explicit)
  if(TEST "GaudiExamples.${test}")
    set_property(
      TEST GaudiExamples.${test}
      APPEND PROPERTY ENVIRONMENT JOBOPTSDUMPFILE=gaudiexamples.${test}.joboptsdump
    )
  endif()
endforeach()

gaudi_add_pytest(tests/pytest)

# Compiled python module
gaudi_add_python_module(PyExample
                        SOURCES src/PythonModule/Functions.cpp
                                src/PythonModule/PyExample.cpp
                        LINK Python::Python
                             Boost::python)

# ROOT dictionaries
gaudi_add_dictionary(GaudiExamplesDict
                     HEADERFILES src/IO/dict.h
                     SELECTION src/IO/dict.xml
                     LINK GaudiExamplesLib)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
configure_file(scripts/TupleEx3.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/generated/scripts/TupleEx3.py)
configure_file(scripts/TupleEx4.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/generated/scripts/TupleEx4.py)
gaudi_install(SCRIPTS)
gaudi_install(SCRIPTS ${CMAKE_CURRENT_BINARY_DIR}/generated/scripts/)
