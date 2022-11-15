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
# GaudiUtils subdirectory

# Build libraries
gaudi_add_library(GaudiUtilsLib
                  SOURCES src/Lib/QuasiRandom.cpp
                  LINK
                    PUBLIC GaudiKernel
                           ROOT::RIO
                           ROOT::Hist
                           ROOT::Thread
                           ROOT::Matrix
                           ROOT::MathCore
                           ROOT::XMLIO)
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
                    src/component/FileReadTool.cpp
                    src/component/SignalMonitorSvc.cpp
                    src/component/IODataManager.cpp
                    src/component/StalledEventMonitor.cpp
                    src/component/MultiFileCatalog.cpp
                    src/component/VFSSvc.cpp
                    src/component/createGuidAsString.cpp
                  LINK
                    GaudiUtilsLib
                    UUID::uuid)

if(GAUDI_USE_XERCESC)
  target_sources(GaudiUtils PRIVATE src/component/XMLCatalogTest.cpp
                                    src/component/XMLFileCatalog.cpp)
  target_link_libraries(GaudiUtils PRIVATE XercesC::XercesC)
endif()

# Build and register tests
if(BUILD_TESTING AND GAUDI_USE_CPPUNIT AND GAUDI_USE_XERCESC)
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
    PROPERTY DEPENDS testXMLFileCatalogWrite)
endif()
