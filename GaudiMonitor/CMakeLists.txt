###############################################
# CMakeLists.txt file for building GaudiMonitor
# @author Pere Mato, CERN
###############################################


INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS} ) 
LINK_DIRECTORIES(${GAUDI_LIBRARY_DIRS})

#---Component Library
GAUDI_COMPONENT_LIBRARY( GaudiMonitor "DataListenerSvc/*.cpp;Dll/GaudiMonitor*.cpp"
                         GaudiKernel)



