############################################################################
# CMakeLists.txt file for building PartPropSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS regex)
find_package(ROOT COMPONENTS Reflex)
find_package(HepPDT)

#---Libraries---------------------------------------------------------------
gaudi_add_module(PartPropSvc *.cpp
                        LINK_LIBRARIES GaudiKernel Boost ROOT HepPDT
                        INCLUDE_DIRS Boost ROOT HepPDT)

#---Installation------------------------------------------------------------
gaudi_install_joboptions(share/PartPropSvc.py)
