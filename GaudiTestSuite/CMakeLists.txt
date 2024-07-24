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
# GaudiTestSuite subdirectory
if(NOT BUILD_TESTING)
  return()
endif()

# Build the library
gaudi_add_library(GaudiTestSuiteLib
                  SOURCES src/Lib/Counter.cpp
                          src/Lib/Event.cpp
                          src/Lib/MyTrack.cpp
                          src/Lib/MyVertex.cpp
                  LINK PUBLIC GaudiKernel)

# Build the plugin
gaudi_add_module(GaudiTestSuiteComponents
                 SOURCES src/AbortEvent/AbortEventAlg.cpp
                         src/AlgSequencer/HelloWorld.cpp
                         src/AlgSequencer/ParentAlg.cpp
                         src/AlgSequencer/StopperAlg.cpp
                         src/AlgSequencer/SubAlg.cpp
                         src/AlgSequencer/TemplatedAlg.cpp
                         src/AlgTools/MyAlgorithm.cpp
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
                         src/FunctionalAlgorithms/EventCounter.cpp
                         src/FunctionalAlgorithms/MakeAndConsume.cpp
                         src/FunctionalAlgorithms/merging_transformer.cpp
                         src/FunctionalAlgorithms/Prescaler.cpp
                         src/FunctionalAlgorithms/selector.cpp
                         src/FunctionalAlgorithms/TestObjectVersion.cpp
                         src/FunctionalAlgorithms/transformer.cpp
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
                         src/NTuple/NTupleWriterProducers.cpp
                         src/NTuple/NTupleWriterImpls.cpp
                 LINK GaudiKernel
                      Gaudi::Functional
                      GaudiUtilsLib
                      GaudiTestSuiteLib
                      ROOT::Tree
                      ROOT::RIO
                      ROOT::Hist
                      ROOT::Net
                      ${GAUDI_ATOMIC_LIBS})

target_sources(GaudiTestSuiteLib PRIVATE src/Lib/legacy_examples.cpp)
target_sources(GaudiTestSuiteComponents PRIVATE src/IO/ExamplesHandleAlgs.cpp)
add_library(GaudiExamplesLib ALIAS GaudiTestSuiteLib)
add_library(${PROJECT_NAME}::GaudiExamplesLib ALIAS GaudiTestSuiteLib)

if(NOT ROOT_VERSION VERSION_LESS "6.20")
  find_package(ROOT REQUIRED COMPONENTS ROOTHist)
  target_link_libraries(GaudiTestSuiteComponents PRIVATE ROOT::ROOTHist)
endif()

if(GAUDI_USE_AIDA)
    target_sources(GaudiTestSuiteComponents PRIVATE
                                         src/Histograms/HistoAlgorithm.cpp
                                         src/Histograms/HistoProps.cpp
                                         src/IO/EvtCollectionSelector.cpp
                                         src/IO/EvtCollectionWrite.cpp
                                         src/IO/EvtExtCollectionSelector.cpp
                                         src/IO/ReadAlg.cpp
                                         src/IO/ReadHandleAlg.cpp
                                         src/IO/ReadTES.cpp
                                         src/IO/WriteAlg.cpp
                                         src/IO/WriteHandleAlg.cpp
                                         src/RandomNumber/RandomNumberAlg.cpp)
    target_link_libraries(GaudiTestSuiteComponents PRIVATE AIDA::aida)
endif()
if(GAUDI_USE_HEPPDT)
    target_sources(GaudiTestSuiteComponents PRIVATE src/PartProp/PartPropExa.cpp)
    target_link_libraries(GaudiTestSuiteComponents PRIVATE HepPDT::heppdt)
endif()
if(TARGET RootCnvLib)
    target_sources(GaudiTestSuiteComponents PRIVATE src/MultiInput/DumpAddress.cpp
                                         src/MultiInput/MIReadAlg.cpp
                                         src/MultiInput/MIWriteAlg.cpp)
    target_link_libraries(GaudiTestSuiteComponents PRIVATE RootCnvLib)
endif()
if(GAUDI_ENABLE_GAUDIPARTPROP)
    target_sources(GaudiTestSuiteComponents PRIVATE
        src/PartProp/GaudiPPS.cpp
        src/PartProp/PartPropAlg.cpp
    )
    target_link_libraries(GaudiTestSuiteComponents PRIVATE Gaudi::GaudiPartProp)
endif()

# Build the executable
gaudi_add_executable(Allocator
                        SOURCES src/Allocator/Allocator.cpp
                                src/Allocator/MyClass1A.cpp
                                src/Allocator/MyClass1.cpp
                        LINK GaudiTestSuiteLib
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

# Generate GaudiTestSuite_user.confdb
gaudi_generate_confuserdb()

# Tests
gaudi_add_tests(QMTest)
if(BUILD_TESTING)
  if(TEST "GaudiTestSuite.jira.gaudi_1174" AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_property(TEST GaudiTestSuite.jira.gaudi_1174 PROPERTY DISABLED TRUE)
  endif()
  foreach(test IN ITEMS
      metadatasvc
      properties.all properties.all2 properties.all_py
      bug_34121_tool_properties_in_python
      deferred_write.enforced deferred_write.explicit)
    if(TEST "GaudiTestSuite.${test}")
      set_property(
        TEST GaudiTestSuite.${test}
        APPEND PROPERTY ENVIRONMENT JOBOPTSDUMPFILE=gauditestsuite.${test}.joboptsdump
      )
    endif()
  endforeach()
  if(NOT GAUDI_ENABLE_GAUDIALG)
    # these tests are for features available in GaudiAlg, but depend on a fixture in GaudiTestSuite tests
    set_tests_properties(
      GaudiTestSuite.root_io.extcoll.write GaudiTestSuite.root_io.extcoll.read
      PROPERTIES DISABLED TRUE
    )
  endif()
  if(NOT GAUDI_ENABLE_GAUDIPARTPROP)
    set_tests_properties(
      GaudiTestSuite.particlepropsvc
      PROPERTIES DISABLED TRUE
    )
  endif()
  if(gperftools_VERSION STREQUAL "2.15")
    # FIXME We get a segfault in tcmalloc when we use gperftools 2.15 from LCG 106
    set_tests_properties(
      GaudiTestSuite.google_auditors.heapchecker
      PROPERTIES DISABLED TRUE
    )
  endif()
endif()

gaudi_add_pytest(tests/pytest)

# Compiled python module
gaudi_add_python_module(PyExample
                        SOURCES src/PythonModule/Functions.cpp
                                src/PythonModule/PyExample.cpp
                        LINK Python::Python
                             Boost::python)

# ROOT dictionaries
gaudi_add_dictionary(GaudiTestSuiteDict
                     HEADERFILES src/IO/dict.h
                     SELECTION src/IO/dict.xml
                     LINK GaudiTestSuiteLib)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)
