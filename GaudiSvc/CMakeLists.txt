###########################################
# CMakeList.txt file for building GaudiSvc
# @author Pere Mato, CERN
###########################################

GAUDI_USE_PACKAGE(CLHEP)
GAUDI_USE_PACKAGE(GaudiKernel)


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

#---This is needed by SET_RUNTIME_PATH() :-(
SET( GAUDI_LIBRARY_DIRS ${GAUDI_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE )




