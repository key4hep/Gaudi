############################################################################
# CMakeLists.txt file for building GaudiUtils
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

find_package(Boost)
find_package(AIDA)
find_package(ROOT)
find_package(XercesC)
find_package(uuid)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiUtils component/*.cpp
                        LIBRARIES GaudiUtilsLib ${UUID_LIBRARIES} ${XERCESC_LIBRARIES}
                        USE_HEADERS Boost ROOT AIDA)
GAUDI_LINKER_LIBRARY(GaudiUtilsLib Lib/*.cpp
                     LIBRARIES GaudiKernel ${ROOT_LIBRARIES} ${ROOT_RIO_LIBRARY} ${ROOT_Hist_LIBRARY}
                     USE_HEADERS Boost ROOT AIDA XercesC uuid)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiUtils)
