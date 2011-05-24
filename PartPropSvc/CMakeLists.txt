############################################################################
# CMakeLists.txt file for building PartPropSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

find_package(Boost COMPONENTS regex)
find_package(ROOT)
find_package(HepPDT)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(PartPropSvc *.cpp
                        LIBRARIES GaudiKernel ${ROOT_Reflex_LIBRARY} ${Boost_LIBRARIES} ${HEPPDT_LIBRARIES}
                        USE_HEADERS Boost ROOT HepPDT)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_JOBOPTIONS(share/PartPropSvc.py)
