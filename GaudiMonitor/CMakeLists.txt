###############################################
# CMakeLists.txt file for building GaudiMonitor
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(GaudiKernel)

#---Component Library
GAUDI_COMPONENT_LIBRARY( GaudiMonitor "DataListenerSvc/*.cpp;Dll/GaudiMonitor*.cpp"
                         GaudiKernel)



