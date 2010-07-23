#############################################
# CMakeList.txt file for building RootHistCnv
# @author Pere Mato, CERN
#############################################

FIND_PACKAGE(ROOT)
FIND_PACKAGE(AIDA)

INCLUDE_DIRECTORIES( ${GAUDI_INCLUDE_DIRS} ${AIDA_INCLUDE_DIRS} ) 
LINK_DIRECTORIES( ${GAUDI_LIBRARY_DIRS} ) 

#---Component Library
GAUDI_COMPONENT_LIBRARY(RootHistCnv *.cpp GaudiKernel Hist Tree Matrix RIO)

