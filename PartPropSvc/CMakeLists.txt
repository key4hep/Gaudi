#############################################
# CMakeList.txt file for building PartPropSvc
# @author Pere Mato, CERN
#############################################

GAUDI_USE_PACKAGE(HepPDT)
GAUDI_USE_PACKAGE(GaudiKernel)

#---Component Library
GAUDI_COMPONENT_LIBRARY(PartPropSvc *.cpp GaudiKernel ${HepPDT_LIBRARIES})

