############################################################################
# CMakeLists.txt file for building GaudiAud
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiAud *.cpp LIBRARIES GaudiKernel)
