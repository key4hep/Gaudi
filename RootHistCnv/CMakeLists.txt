#############################################
# CMakeList.txt file for building RootHistCnv
# @author Pere Mato, CERN
#############################################

GAUDI_USE_PACKAGE(GaudiKernel)

#---Component Library
GAUDI_COMPONENT_LIBRARY(RootHistCnv *.cpp GaudiKernel Hist Tree Matrix RIO)

