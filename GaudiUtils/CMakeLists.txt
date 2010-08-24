############################################################################
# CMakeLists.txt file for building GaudiUtils
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(ROOT)
GAUDI_USE_PACKAGE(AIDA)
GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(uuid)
GAUDI_USE_PACKAGE(XercesC)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiUtils component/*.cpp GaudiUtilsLib ${uuid_LIBRARIES} ${XercesC_LIBRARIES})
GAUDI_LINKER_LIBRARY(GaudiUtilsLib Lib/*.cpp GaudiKernel ${ROOT_LIBRARIES} RIO Hist)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiUtils)
