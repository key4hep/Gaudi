###########################################
# CMakeList.txt file for building GaudiAud
# @author Pere Mato, CERN
###########################################

INCLUDE_DIRECTORIES( ${GAUDI_INCLUDE_DIRS} ) 
LINK_DIRECTORIES( ${GAUDI_LIBRARY_DIRS} ) 

#---Component Library

GAUDI_COMPONENT_LIBRARY(GaudiAud *.cpp GaudiKernel)

