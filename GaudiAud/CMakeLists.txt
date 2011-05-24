############################################################################
# CMakeLists.txt file for building GaudiAud
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

# FIXME: In principle these should not be needed because they should be implicit
#        in the usage of the GaudiKernel library
find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiAud *.cpp LIBRARIES GaudiKernel)


