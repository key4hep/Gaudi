############################################################################
# CMakeLists.txt file for building GaudiUtils
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

find_package(XercesC)
include_directories(${XERCESC_INCLUDE_DIRS})

find_package(uuid)
include_directories(${UUID_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiUtils component/*.cpp LIBRARIES GaudiUtilsLib ${UUID_LIBRARIES} ${XERCESC_LIBRARIES})
GAUDI_LINKER_LIBRARY(GaudiUtilsLib Lib/*.cpp LIBRARIES GaudiKernel ${ROOT_LIBRARIES}
                     ${ROOT_RIO_LIBRARY} ${ROOT_Hist_LIBRARY})

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiUtils)
