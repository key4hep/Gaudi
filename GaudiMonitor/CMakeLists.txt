############################################################################
# CMakeLists.txt file for building GaudiMonitor
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)

if(WIN32)
  set(winsocklib ws2_32.lib)
endif()
#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiMonitor DataListenerSvc/*.cpp LIBRARIES GaudiKernel ${winsocklib})

