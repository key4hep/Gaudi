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
gaudi_subdir(GaudiUtils)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost REQUIRED)
find_package(AIDA)
set (XMLIO_PACKAGE_NAME XMLIO)
if (${APPLE})
  set (XMLIO_PACKAGE_NAME "")
endif (${APPLE})
find_package(ROOT COMPONENTS RIO Hist ${XMLIO_PACKAGE_NAME} Thread Matrix MathCore REQUIRED)
find_package(XercesC)
find_package(UUID REQUIRED)
find_package(CppUnit)

# Decide whether to link against AIDA:
set( aida_lib )
if( AIDA_FOUND )
   set( aida_lib AIDA )
endif()

# Decide whether to link against XercesC:
set( xercesc_lib )
if( XERCESC_FOUND )
   set( xercesc_lib XercesC )
endif()

# The list of sources to build into the library:
set( lib_sources src/Lib/QuasiRandom.cpp )
if( AIDA_FOUND )
   list( APPEND lib_sources src/Lib/Aida2ROOT.cpp src/Lib/HistoLabels.cpp
      src/Lib/HistoStrings.cpp src/Lib/Histo2String.cpp src/Lib/HistoParsers.cpp
      src/Lib/HistoTableFormat.cpp src/Lib/HistoDump.cpp src/Lib/HistoStats.cpp
      src/Lib/HistoXML.cpp )
endif()

# The list of sources to build into the module:
set( module_sources src/component/FileReadTool.cpp
   src/component/SignalMonitorSvc.cpp src/component/IODataManager.cpp
   src/component/StalledEventMonitor.cpp src/component/MultiFileCatalog.cpp
   src/component/VFSSvc.cpp src/component/createGuidAsString.cpp )
if( XERCESC_FOUND )
   list( APPEND module_sources src/component/XMLCatalogTest.cpp
      src/component/XMLFileCatalog.cpp )
endif()

# Hide some Boost/ROOT/XercesC compile time warnings
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} )
if( XERCESC_FOUND )
   include_directories( SYSTEM ${XERCESC_INCLUDE_DIRS} )
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiUtilsLib ${lib_sources}
                  LINK_LIBRARIES GaudiKernel ROOT
                  INCLUDE_DIRS ROOT ${aida_lib}
                  PUBLIC_HEADERS GaudiUtils)
gaudi_add_module(GaudiUtils ${module_sources}
                 LINK_LIBRARIES GaudiUtilsLib ${xercesc_lib} UUID
                 INCLUDE_DIRS ${xercesc_lib} UUID)

if( CPPUNIT_FOUND )
   gaudi_add_unit_test(testXMLFileCatalogWrite
      src/component/createGuidAsString.cpp
      src/component/XMLCatalogTest.cpp
      src/component/XMLFileCatalog.cpp
      LINK_LIBRARIES GaudiKernel XercesC UUID
      INCLUDE_DIRS XercesC UUID)
   set_property(TARGET testXMLFileCatalogWrite
      APPEND PROPERTY COMPILE_DEFINITIONS testXMLFileCatalogWrite=main)

   gaudi_add_unit_test(testXMLFileCatalogRead
      src/component/createGuidAsString.cpp
      src/component/XMLCatalogTest.cpp
      src/component/XMLFileCatalog.cpp
      LINK_LIBRARIES GaudiKernel XercesC UUID
      INCLUDE_DIRS XercesC UUID)
   set_property(TARGET testXMLFileCatalogRead
      APPEND PROPERTY COMPILE_DEFINITIONS testXMLFileCatalogRead=main)

   set_property(TEST GaudiUtils.testXMLFileCatalogRead
      PROPERTY DEPENDS GaudiUtils.testXMLFileCatalogWrite)
endif()
