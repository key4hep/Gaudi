############################################################################
# CMakeLists.txt file for building GaudiMonitor
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiMonitor DataListenerSvc/*.cpp GaudiKernel)

