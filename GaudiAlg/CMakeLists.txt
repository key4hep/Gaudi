############################################################################
# CMakeLists.txt file for building GaudiAlg
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

find_package(Boost COMPONENTS regex)
include_directories(${Boost_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiAlgLib lib/*.cpp LIBRARIES GaudiUtilsLib ${Boost_LIBRARIES})
GAUDI_COMPONENT_LIBRARY(GaudiAlg components/*.cpp LIBRARIES GaudiAlgLib GaudiKernel ${ROOT_LIBRARIES} ${ROOT_RIO_LIBRARY})

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiAlg)
