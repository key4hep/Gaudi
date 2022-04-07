#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# GaudiKernel subdirectory

option(GAUDI_V21 "disable Gaudi v20 compatibiility" OFF)
option(GAUDI_V22 "enable Gaudi v22 new APIs (implies GAUDI_V21=ON)" OFF)

# Look for VectorClass required header (private dependency)
find_path(VectorClass_INCLUDE_DIR NAMES instrset_detect.cpp)
if(VectorClass_INCLUDE_DIR AND NOT VectorClass_VERSION)
  # check that the version is good enough
  set(VectorClass_VERSION 0.0)
  file(STRINGS ${VectorClass_INCLUDE_DIR}/instrset.h _vectorclass_guard REGEX "define +INSTRSET_H +[0-9]+")
  list(GET _vectorclass_guard 0 _vectorclass_guard)
  if(_vectorclass_guard MATCHES "INSTRSET_H +([0-9][0-9][0-9][0-9][0-9])")
    string(REGEX REPLACE "([0-9]+)([0-9][0-9])([0-9][0-9])" "\\1.\\2.\\3" VectorClass_VERSION "${CMAKE_MATCH_1}")
  elseif(_vectorclass_guard MATCHES "INSTRSET_H +([0-9][0-9][0-9])")
    string(REGEX REPLACE "([0-9]+)([0-9][0-9])" "\\1.\\2" VectorClass_VERSION "${CMAKE_MATCH_1}")
  endif()
  set(VectorClass_VERSION "${VectorClass_VERSION}" CACHE INTERNAL "")
endif()
if(NOT VectorClass_INCLUDE_DIR OR VectorClass_VERSION VERSION_LESS 2.01.02)
  if(VectorClass_INCLUDE_DIR)
    message(STATUS "Found VectorClass instrset_detect ${VectorClass_VERSION} at ${VectorClass_INCLUDE_DIR}")
  endif()
  message(WARNING "using internal VectorClass instrset_detect (2.01.02)")
  set(VectorClass_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/contrib" CACHE PATH "" FORCE)
  set(VectorClass_VERSION "1.25" CACHE INTERNAL "" FORCE)
endif()
mark_as_advanced(VectorClass_INCLUDE_DIR)
message(STATUS "Using VectorClass instrset_detect ${VectorClass_VERSION} at ${VectorClass_INCLUDE_DIR}")

# Build the library
gaudi_add_library(GaudiKernel
  SOURCES src/Lib/AlgContext.cpp
          src/Lib/Algorithm.cpp
          src/Lib/AlgorithmHistory.cpp
          src/Lib/AlgTool.cpp
          src/Lib/AlgToolHistory.cpp
          src/Lib/AllocatorPool.cpp
          src/Lib/Application.cpp
          src/Lib/Auditor.cpp
          src/Lib/Bootstrap.cpp
          src/Lib/ChronoEntity.cpp
          src/Lib/ComponentManager.cpp
          src/Lib/ConcurrencyFlags.cpp
          src/Lib/ContainedObject.cpp
          src/Lib/ConversionSvc.cpp
          src/Lib/Converter.cpp
          src/Lib/DataHandle.cpp
          src/Lib/DataHandleFinder.cpp
          src/Lib/DataHandleHolderVisitor.cpp
          src/Lib/DataHistory.cpp
          src/Lib/DataObject.cpp
          src/Lib/DataObjectHandleBase.cpp
          src/Lib/DataHandleProperty.cpp
          src/Lib/DataObjID.cpp
          src/Lib/DataStreamTool.cpp
          src/Lib/DataSvc.cpp
          src/Lib/DataTypeInfo.cpp
          src/Lib/Debugger.cpp
          src/Lib/Dictionary.cpp
          src/Lib/DirSearchPath.cpp
          src/Lib/Environment.cpp
          src/Lib/EventIDBase.cpp
          src/Lib/EventIDRange.cpp
          src/Lib/EventSelectorDataStream.cpp
          src/Lib/FunctionalFilterDecision.cpp
          src/Lib/GaudiException.cpp
          src/Lib/GaudiHandle.cpp
          src/Lib/GaudiMain.cpp
          src/Lib/Guards.cpp
          src/Lib/HistoDef.cpp
          src/Lib/HistoryObj.cpp
          src/Lib/IChronoStatSvc.cpp
          src/Lib/IConversionSvc.cpp
          src/Lib/IDataProviderSvc.cpp
          src/Lib/IInterface.cpp
          src/Lib/Incident.cpp
          src/Lib/IPartitionControl.cpp
          src/Lib/ISvcLocator.cpp
          src/Lib/ITimelineSvc.cpp
          src/Lib/JobHistory.cpp
          src/Lib/KeyedObjectManager.cpp
          src/Lib/LegacyAlgorithm.cpp
          src/Lib/LinkManager.cpp
          src/Lib/MapBase.cpp
          src/Lib/Memory.cpp
          src/Lib/Message.cpp
          src/Lib/MinimalEventLoopMgr.cpp
          src/Lib/ModuleInfo.cpp
          src/Lib/MsgStream.cpp
          src/Lib/NTupleImplementation.cpp
          src/Lib/NTupleItems.cpp
          src/Lib/ParsersCollections.cpp
          src/Lib/ParsersHistograms.cpp
          src/Lib/ParsersStandardList1.cpp
          src/Lib/ParsersStandardList2.cpp
          src/Lib/ParsersStandardList3.cpp
          src/Lib/ParsersStandardList4.cpp
          src/Lib/ParsersStandardMisc1.cpp
          src/Lib/ParsersStandardMisc2.cpp
          src/Lib/ParsersStandardMisc3.cpp
          src/Lib/ParsersStandardMisc4.cpp
          src/Lib/ParsersStandardMisc5.cpp
          src/Lib/ParsersStandardSingle.cpp
          src/Lib/ParsersVct.cpp
          src/Lib/PathResolver.cpp
          src/Lib/PrintAlgsSequences.cpp
          src/Lib/ProcessDescriptor.cpp
          src/Lib/Property.cpp
          src/Lib/PropertyMgr.cpp
          src/Lib/Range.cpp
          src/Lib/RegistryEntry.cpp
          src/Lib/RenounceToolInputsVisitor.cpp
          src/Lib/RndmGenerators.cpp
          src/Lib/RndmTypeInfos.cpp
          src/Lib/Selector.cpp
          src/Lib/Sequence.cpp
          src/Lib/Service.cpp
          src/Lib/ServiceHistory.cpp
          src/Lib/ServiceLocatorHelper.cpp
          src/Lib/Sleep.cpp
          src/Lib/SmartDataObjectPtr.cpp
          src/Lib/SmartRefBase.cpp
          src/Lib/SmartRefVector.cpp
          src/Lib/Stat.cpp
          src/Lib/StateMachine.cpp
          src/Lib/StatusCode.cpp
          src/Lib/StringKey.cpp
          src/Lib/System.cpp
          src/Lib/ThreadLocalContext.cpp
          src/Lib/Time.cpp
          src/Lib/Timing.cpp
          src/Lib/ToolVisitor.cpp
          src/Lib/TsDataSvc.cpp
          src/Lib/VirtualDestructors.cpp
          src/Lib/WatchdogThread.cpp
  LINK
    PUBLIC GaudiPluginService
           Boost::headers
           Boost::filesystem
           Boost::thread
           Boost::regex
           Boost::system
           TBB::tbb
           ${CMAKE_DL_LIBS}
           ROOT::Core
           cppgsl::cppgsl
           fmt::fmt
           nlohmann_json::nlohmann_json
           ${GAUDI_ATOMIC_LIBS}
    PRIVATE Python::Python
)
gaudi_generate_version_header_file(GaudiKernel)
target_include_directories(GaudiKernel
  PRIVATE ${VectorClass_INCLUDE_DIR}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>  # for GAUDI_VERSION.h
  )
if(APPLE)
  target_sources(GaudiKernel PRIVATE src/Lib/Platform/SystemMacOS.h
                                     src/Lib/Platform/SystemMacOS.mm)
  target_link_libraries(GaudiKernel PUBLIC "-framework Foundation -framework CoreFoundation")
elseif(WIN32)
  target_sources(GaudiKernel PRIVATE src/Lib/Platform/SystemWin32.h
                                     src/Lib/Platform/SystemWin32.cpp)
elseif(UNIX)
  target_sources(GaudiKernel PRIVATE src/Lib/Platform/SystemLinux.h
                                     src/Lib/Platform/SystemLinux.cpp)
  target_link_libraries(GaudiKernel PUBLIC ${rt_LIBRARY})
endif()

if(GAUDI_v22)
  set(GAUDI_V21 TRUE)
  target_compile_definitions(GaudiKernel PUBLIC GAUDI_V22_API)
endif()
if(NOT GAUDI_V21)
  target_compile_definitions(GaudiKernel PUBLIC GAUDI_V20_COMPAT)
endif()

# Build an executable
gaudi_add_executable(profile_Property
                     SOURCES tests/src/profile_Property.cpp
                     LINK GaudiKernel)

# Build and register tests
get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
gaudi_add_executable(DirSearchPath_test
                     SOURCES tests/src/DirSearchPath_test.cpp
                     LINK GaudiKernel
                     TEST)

gaudi_add_executable(PathResolver_test SOURCES tests/src/PathResolver_test.cpp
  LINK GaudiKernel)
add_test(NAME ${package_name}.PathResolver_test
         COMMAND run $<TARGET_FILE:PathResolver_test>
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

gaudi_add_executable(Parsers_test SOURCES tests/src/parsers.cpp
  LINK GaudiKernel TEST)

gaudi_add_executable(Memory_test SOURCES tests/src/Memory_test.cpp
  LINK GaudiKernel TEST)

gaudi_add_executable(ContextSpecificPtrTest SOURCES tests/src/test_ContextSpecificPtr.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_GAUDI-905 SOURCES tests/src/GAUDI-905.cpp
  LINK GaudiKernel TEST)

gaudi_add_executable(test_GAUDI-973 SOURCES tests/src/GAUDI-973.cpp
  LINK GaudiKernel TEST)

gaudi_add_executable(test_PropertyHolder SOURCES tests/src/test_PropertyHolder.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_Property SOURCES tests/src/test_Property.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_WeakPropertyRef SOURCES tests/src/test_WeakPropertyRef.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_StatusCode SOURCES tests/src/test_StatusCode.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_EventIDBase SOURCES tests/src/test_EventIDBase.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_EventIDRange SOURCES tests/src/test_EventIDRange.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_SystemTypeinfoName SOURCES tests/src/test_SystemTypeinfoName.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_SystemCmdLineArgs SOURCES tests/src/test_SystemCmdLineArgs.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_compose SOURCES tests/src/test_compose.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_reverse SOURCES tests/src/test_reverse.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_Counters SOURCES tests/src/CountersUnitTest.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_CounterHistos SOURCES tests/src/CounterHistosUnitTest.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_GenericSink SOURCES tests/src/GenericSinkUnitTest.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(AttribStringParser_test SOURCES tests/src/test_AttribStringParser.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_MonotonicArena SOURCES tests/src/test_MonotonicArena.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

gaudi_add_executable(test_GaudiTimer SOURCES tests/src/test_GaudiTimer.cpp
  LINK GaudiKernel Boost::unit_test_framework TEST)

foreach(test_case IN ITEMS 01 02 03 04)
  add_executable(test_StatusCodeFail_case${test_case} tests/src/test_StatusCode_fail.cxx)
  target_include_directories(test_StatusCodeFail_case${test_case} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(test_StatusCodeFail_case${test_case} PRIVATE Boost::headers)
  set_target_properties(test_StatusCodeFail_case${test_case} PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  target_compile_definitions(test_StatusCodeFail_case${test_case} PRIVATE CASE${test_case})
  add_test(NAME ${package_name}.test_StatusCodeFail.case${test_case}
           COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test_StatusCodeFail_case${test_case})
  set_property(TEST ${package_name}.test_StatusCodeFail.case${test_case} PROPERTY PASS_REGULAR_EXPRESSION "FAIL${test_case}")
  set_property(TEST ${package_name}.test_StatusCodeFail.case${test_case} PROPERTY RESOURCE_LOCK build-system)
endforeach()

if(GAUDI_USE_CPPUNIT)
  gaudi_add_executable(test_SerializeSTL SOURCES tests/src/test_SerializeSTL.cpp
    LINK GaudiKernel CppUnit::cppunit TEST)

  gaudi_add_executable(test_AnyDataObject SOURCES tests/src/test_AnyDataObject.cpp
    LINK GaudiKernel CppUnit::cppunit TEST)

  gaudi_add_executable(test_DataHandleVector SOURCES tests/src/test_DataHandleVector.cpp
    LINK GaudiKernel CppUnit::cppunit TEST)

  gaudi_add_executable(test_GaudiTime SOURCES tests/src/test_GaudiTime.cpp
    LINK GaudiKernel CppUnit::cppunit TEST)

  gaudi_add_executable(test_GaudiTiming SOURCES tests/src/test_GaudiTiming.cpp
    LINK GaudiKernel CppUnit::cppunit TEST)

  gaudi_add_executable(test_LockedHandle SOURCES tests/src/test_LockedHandle.cpp
    LINK GaudiKernel CppUnit::cppunit Boost::thread ${CMAKE_DL_LIBS} TEST)
endif()

# nosetests
gaudi_add_tests(nosetests ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose/confdb)
gaudi_add_tests(nosetests)

gaudi_add_module(test_CustomFactory
  SOURCES tests/src/custom_factory.cpp
  LINK GaudiPluginService)

# Genconf
gaudi_add_executable(genconf
  SOURCES src/Util/genconf.cpp
  LINK GaudiKernel
       GaudiPluginService
       Boost::headers
       Boost::program_options
       Boost::regex
       Boost::log
       Boost::log_setup
       ROOT::Core)
target_compile_definitions(genconf PRIVATE BOOST_LOG_DYN_LINK)
set_target_properties(genconf PROPERTIES ENABLE_EXPORTS TRUE)
if(WIN32)
  gaudi_add_executable(genwindef
    SOURCES src/Util/genwindef.cpp
            src/Util/LibSymbolInfo.cpp)
endif()
gaudi_add_executable(instructionsetLevel
  SOURCES src/Util/instructionsetLevel.cpp
  LINK GaudiKernel
       GaudiPluginService)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/custom_factory_testdir)
add_test(NAME ${package_name}.genconf_with_custom_factory
  COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=$<TARGET_FILE_DIR:test_CustomFactory>:$<TARGET_FILE_DIR:GaudiCoreSvc>:$ENV{LD_LIBRARY_PATH}
          $<TARGET_FILE:genconf>
          -o ${CMAKE_CURRENT_BINARY_DIR}/custom_factory_testdir
          -p CustomFactoryTest
          -i test_CustomFactory
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/custom_factory_testdir)

# QMTest
gaudi_add_tests(QMTest)

# ROOT dictionaries
gaudi_add_dictionary(GaudiKernelDict
                     HEADERFILES dict/dictionary.h
                     SELECTION dict/dictionary.xml
                     LINK GaudiKernel)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)

# Check that gdb can be used for stack traces
find_program(gdb_cmd gdb)
mark_as_advanced(gdb_cmd)
if(gdb_cmd)
  get_filename_component(gdb_dir "${gdb_cmd}" DIRECTORY)
  file(TO_CMAKE_PATH "$ENV{PATH}" envpath)
  if(NOT gdb_dir IN_LIST envpath)
    set_property(TARGET target_runtime_paths APPEND
      PROPERTY runtime_path $<SHELL_PATH:${gdb_dir}>)
  endif()
else()
  message(WARNING "gdb not found, you will not be able to have stack traces for problems")
endif()
