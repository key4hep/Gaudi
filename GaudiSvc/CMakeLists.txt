###########################################
# CMakeList.txt file for building GaudiSvc
# @author Pere Mato, CERN
###########################################

GAUDI_USE_PACKAGE(CLHEP)

INCLUDE_DIRECTORIES( ${GAUDI_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ) 
LINK_DIRECTORIES( ${GAUDI_LIBRARY_DIRS} ${CLHEP_LIBRARY_DIRS} ) 

SET( srcs AlgContextSvc/*.cpp 
          ApplicationMgr/*.cpp 
          AuditorSvc/*.cpp
          ChronoStatSvc/*.cpp
          EventSelector/*.cpp
          ExceptionSvc/*.cpp
          DataSvc/*.cpp
	  DetectorDataSvc/*.cpp
	  HistogramSvc/*.cpp
	  HistorySvc/*.cpp
          IncidentSvc/*.cpp
          IssueLogger/*.cpp
          JobOptionsSvc/*.cpp
          MessageSvc/*.cpp
          NTupleSvc/*.cpp
          ParticlePropertySvc/*.cpp
          PersistencySvc/*.cpp
          RndmGenSvc/*.cpp
          StatusCodeSvc/*.cpp
          ToolSvc/*.cpp
          THistSvc/*.cpp
          FastContainersSvc/*.cpp
          VFSSvc/*.cpp
          Dll/GaudiSvc*.cpp )
#---Component Library

GAUDI_COMPONENT_LIBRARY(GaudiSvc "${srcs}" GaudiKernel ${ROOT_LIBRARIES} Hist RIO Tree ${CLHEP_LIBRARIES})

SET( GAUDI_LIBRARY_DIRS ${GAUDI_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE )




