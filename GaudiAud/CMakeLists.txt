###########################################
# CMakeList.txt file for building GaudiAud
# @author Pere Mato, CERN
###########################################

GAUDI_USE_PACKAGE(GaudiKernel)

#---Component Library

GAUDI_COMPONENT_LIBRARY(GaudiAud *.cpp GaudiKernel)

