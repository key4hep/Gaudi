############################################################################
# CMakeLists.txt file for building PartPropSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiSvc)
GAUDI_USE_PACKAGE(HepPDT)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(PartPropSvc *.cpp GaudiKernel HepPDT HepPID)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(PartPropSvc)
