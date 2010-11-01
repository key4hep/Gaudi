############################################################################
# CMakeLists.txt file for building PartPropSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(HepPDT)
GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiSvc)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(PartPropSvc *.cpp LIBRARIES GaudiKernel Reflex ${Boost_LIBRARIES} ${Boost_regex} ${HepPDT_LIBRARIES})

#---Installation------------------------------------------------------------
GAUDI_INSTALL_JOBOPTIONS(share/PartPropSvc.py)

