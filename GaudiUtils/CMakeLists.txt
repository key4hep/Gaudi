############################################################################
# CMakeLists.txt file for building GaudiUtils
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(GaudiSvc)
#G AUDI_USE_PACKAGE(ROOT)
#G AUDI_USE_PACKAGE(AIDA)
#G AUDI_USE_PACKAGE(uuid)
#G AUDI_USE_PACKAGE(XercesC)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

find_package(XercesC)
include_directories(${XercesC_INCLUDE_DIRS})

find_package(uuid)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiUtils component/*.cpp LIBRARIES GaudiUtilsLib ${uuid_LIBRARIES} ${XercesC_LIBRARIES})
GAUDI_LINKER_LIBRARY(GaudiUtilsLib Lib/*.cpp LIBRARIES GaudiKernel ${ROOT_LIBRARIES}
                     ${ROOT_RIO_LIBRARY} ${ROOT_Hist_LIBRARY})

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiUtils)
