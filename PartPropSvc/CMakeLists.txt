############################################################################
# CMakeLists.txt file for building PartPropSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
#G AUDI_USE_PACKAGE(HepPDT)
#G AUDI_USE_PACKAGE(Boost)
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(GaudiSvc)

find_package(Boost COMPONENTS regex)
include_directories(${Boost_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

find_package(HepPDT)
include_directories(${HepPDT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(PartPropSvc *.cpp LIBRARIES GaudiKernel ${ROOT_Reflex_LIBRARY} ${Boost_LIBRARIES} ${HepPDT_LIBRARIES})

#---Installation------------------------------------------------------------
GAUDI_INSTALL_JOBOPTIONS(share/PartPropSvc.py)
