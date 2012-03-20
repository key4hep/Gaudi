############################################################################
# CMakeLists.txt file for building GaudiUtils
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

find_package(Boost)
find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist XMLIO)
find_package(XercesC)
find_package(uuid)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiUtilsLib Lib/*.cpp
                  LINK_LIBRARIES GaudiKernel ROOT
                  INCLUDE_DIRS ROOT AIDA
                  PUBLIC_HEADERS GaudiUtils)
gaudi_add_module(GaudiUtils component/*.cpp
                 LINK_LIBRARIES GaudiUtilsLib XercesC uuid
                 INCLUDE_DIRS XercesC uuid)
