############################################################################
# CMakeLists.txt file for building GaudiAlg
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiUtils GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS regex)
find_package(ROOT COMPONENTS RIO)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiAlgLib lib/*.cpp
                     LINK_LIBRARIES GaudiUtilsLib ${Boost_LIBRARIES}
                     INCLUDE_DIRS Boost AIDA)
gaudi_add_module(GaudiAlg components/*.cpp
                        LINK_LIBRARIES GaudiAlgLib GaudiKernel ${ROOT_LIBRARIES}
                        INCLUDE_DIRS Boost AIDA)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiAlg)
