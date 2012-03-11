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
gaudi_add_library(GaudiUtilsLib Lib/*.cpp
                     LINK_LIBRARIES GaudiKernel ${ROOT_LIBRARIES} ${ROOT_RIO_LIBRARY} ${ROOT_Hist_LIBRARY} ${ROOT_XMLIO_LIBRARY}
                     INCLUDE_DIRS ROOT AIDA)
gaudi_add_module(GaudiUtils component/*.cpp
                        LINK_LIBRARIES GaudiUtilsLib ${UUID_LIBRARIES} ${XERCESC_LIBRARIES}
                        INCLUDE_DIRS XercesC uuid)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiUtils)
