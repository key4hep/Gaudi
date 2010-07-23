#############################################
# CMakeList.txt file for building PartPropSvc
# @author Pere Mato, CERN
#############################################

FIND_PACKAGE(HepPDT)

INCLUDE_DIRECTORIES( ${GAUDI_INCLUDE_DIRS} ${HepPDT_INCLUDE_DIRS}) 
LINK_DIRECTORIES( ${GAUDI_LIBRARY_DIRS} ${HepPDT_LIBRARY_DIRS} ) 

#---Component Library
GAUDI_COMPONENT_LIBRARY(PartPropSvc *.cpp GaudiKernel ${HepPDT_LIBRARIES})

