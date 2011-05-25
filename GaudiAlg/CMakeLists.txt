############################################################################
# CMakeLists.txt file for building GaudiAlg
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiUtils GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS regex)

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiAlgLib lib/*.cpp
                     LIBRARIES GaudiUtilsLib ${Boost_LIBRARIES}
                     USE_HEADERS Boost AIDA)
GAUDI_COMPONENT_LIBRARY(GaudiAlg components/*.cpp
                        LIBRARIES GaudiAlgLib GaudiKernel ${ROOT_LIBRARIES} ${ROOT_RIO_LIBRARY}
                        USE_HEADERS Boost AIDA)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiAlg)
