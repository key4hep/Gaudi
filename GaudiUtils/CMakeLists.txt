#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# GaudiUtils subdirectory

# Build libraries
gaudi_add_library(GaudiUtilsLib
                  SOURCES
                      src/Lib/QuasiRandom.cpp
                      src/Lib/BranchWrapper.cpp
                      src/Lib/PeriodicAction.cpp
                  LINK
                    PUBLIC GaudiKernel
                           Gaudi::Functional
                           ROOT::RIO
                           ROOT::Hist
                           ROOT::Thread
                           ROOT::Matrix
                           ROOT::MathCore
                           ROOT::Tree
                           ROOT::XMLIO
                           Threads::Threads
)
if(GAUDI_USE_AIDA)
  target_sources(GaudiUtilsLib PRIVATE
                  src/Lib/Aida2ROOT.cpp
                  src/Lib/HistoLabels.cpp
                  src/Lib/HistoStrings.cpp
                  src/Lib/Histo2String.cpp
                  src/Lib/HistoParsers.cpp
                  src/Lib/HistoTableFormat.cpp
                  src/Lib/HistoDump.cpp
                  src/Lib/HistoStats.cpp
                  src/Lib/HistoXML.cpp)
  target_link_libraries(GaudiUtilsLib PUBLIC AIDA::aida)
endif()

gaudi_add_module(GaudiUtils
                  SOURCES
                    src/component/EventWatchdogAlg.cpp
                    src/component/FileReadTool.cpp
                    src/component/SignalMonitorSvc.cpp
                    src/component/IODataManager.cpp
                    src/component/StalledEventMonitor.cpp
                    src/component/MultiFileCatalog.cpp
                    src/component/VFSSvc.cpp
                    src/component/createGuidAsString.cpp
                    src/component/GenericNTupleWriter.cpp
                  LINK
                    GaudiUtilsLib
                    Gaudi::Functional
                    UUID::uuid
                    Rangev3::rangev3
                    ROOT::Tree)

if(GAUDI_USE_XERCESC)
  target_sources(GaudiUtils PRIVATE src/component/XMLCatalogTest.cpp
                                    src/component/XMLFileCatalog.cpp)
  target_link_libraries(GaudiUtils PRIVATE XercesC::XercesC)
endif()

# Build and register tests
if(BUILD_TESTING)
  if(GAUDI_USE_CPPUNIT AND GAUDI_USE_XERCESC)
    get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    gaudi_add_executable(testXMLFileCatalogWrite
                          SOURCES
                            src/component/createGuidAsString.cpp
                            src/component/XMLCatalogTest.cpp
                            src/component/XMLFileCatalog.cpp
                          LINK
                            GaudiUtilsLib
                            GaudiKernel
                            UUID::uuid
                            XercesC::XercesC)
    set_property(TARGET testXMLFileCatalogWrite
      APPEND PROPERTY COMPILE_DEFINITIONS testXMLFileCatalogWrite=main)
    add_test(NAME ${package_name}.testXMLFileCatalogWrite
            COMMAND run $<TARGET_FILE:testXMLFileCatalogWrite>)

    gaudi_add_executable(testXMLFileCatalogRead
                          SOURCES
                            src/component/createGuidAsString.cpp
                            src/component/XMLCatalogTest.cpp
                            src/component/XMLFileCatalog.cpp
                          LINK
                            GaudiUtilsLib
                            GaudiKernel
                            UUID::uuid
                            XercesC::XercesC)
    set_property(TARGET testXMLFileCatalogRead
      APPEND PROPERTY COMPILE_DEFINITIONS testXMLFileCatalogRead=main)
    add_test(NAME ${package_name}.testXMLFileCatalogRead
            COMMAND run $<TARGET_FILE:testXMLFileCatalogRead>)
    set_property(TEST ${package_name}.testXMLFileCatalogRead
      PROPERTY DEPENDS ${package_name}.testXMLFileCatalogWrite)
  endif()

  gaudi_add_executable(test_GenericNTupleWriter
        SOURCES
            src/component/GenericNTupleWriter.cpp
        LINK
            GaudiUtilsLib
            Boost::headers
            Boost::unit_test_framework
            fmt::fmt
            Gaudi::GaudiKernel
            ROOT::Tree
        TEST
    )
    target_compile_definitions(test_GenericNTupleWriter PRIVATE UNIT_TESTS)

  gaudi_add_executable(GaudiUtilsTests
      SOURCES
          src/tests/PeriodicAction.cpp
      LINK
          Catch2::Catch2WithMain
          GaudiUtilsLib
  )
  target_include_directories(GaudiUtilsTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_compile_definitions(GaudiUtilsTests PRIVATE BUILD_UNIT_TESTS)
  catch_discover_tests(GaudiUtilsTests
      TEST_PREFIX GaudiUtils.
      PROPERTIES
          LABELS "Gaudi"
          LABELS "Gaudi.GaudiUtils"
  )
endif()
