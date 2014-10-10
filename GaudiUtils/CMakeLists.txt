gaudi_subdir(GaudiUtils v5r3)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost)
find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist XMLIO)
find_package(XercesC)
find_package(UUID)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiUtilsLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiKernel ROOT
                  INCLUDE_DIRS ROOT AIDA
                  PUBLIC_HEADERS GaudiUtils)
gaudi_add_module(GaudiUtils src/component/*.cpp
                 LINK_LIBRARIES GaudiUtilsLib XercesC UUID
                 INCLUDE_DIRS XercesC UUID)

gaudi_add_unit_test(testXMLFileCatalogWrite
                      src/component/XMLCatalogTest.cpp
                      src/component/XMLFileCatalog.cpp
                    LINK_LIBRARIES GaudiKernel XercesC UUID
                    INCLUDE_DIRS XercesC UUID)
set_property(TARGET testXMLFileCatalogWrite
             APPEND PROPERTY COMPILE_DEFINITIONS testXMLFileCatalogWrite=main)

gaudi_add_unit_test(testXMLFileCatalogRead
                      src/component/XMLCatalogTest.cpp
                      src/component/XMLFileCatalog.cpp
                    LINK_LIBRARIES GaudiKernel XercesC UUID
                    INCLUDE_DIRS XercesC UUID)
set_property(TARGET testXMLFileCatalogRead
             APPEND PROPERTY COMPILE_DEFINITIONS testXMLFileCatalogRead=main)

set_property(TEST GaudiUtils.testXMLFileCatalogRead
             PROPERTY DEPENDS GaudiUtils.testXMLFileCatalogWrite)
