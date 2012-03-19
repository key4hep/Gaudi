############################################################################
# CMakeLists.txt file for building PartPropSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

find_package(Boost COMPONENTS regex)
find_package(ROOT COMPONENTS Reflex)
find_package(HepPDT)

#---Libraries---------------------------------------------------------------
gaudi_add_module(PartPropSvc *.cpp
                        LINK_LIBRARIES GaudiKernel Boost ROOT HepPDT
                        INCLUDE_DIRS Boost ROOT HepPDT)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_JOBOPTIONS(share/PartPropSvc.py)
