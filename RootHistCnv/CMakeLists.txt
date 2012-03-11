############################################################################
# CMakeLists.txt file for building RootHistCnv
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist Matrix Tree)
find_package(Boost)

#---Libraries---------------------------------------------------------------
gaudi_add_module(RootHistCnv *.cpp
                        LINK_LIBRARIES GaudiKernel ${ROOT_LIBRARIES}
                        INCLUDE_DIRS Boost ROOT AIDA)
