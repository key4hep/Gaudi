############################################################################
# CMakeLists.txt file for building GaudiSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(CLHEP)
GAUDI_USE_PACKAGE(AIDA)
GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(ROOT)

#---Libraries---------------------------------------------------------------
SET( GaudiSvc_srcs AlgContextSvc/*.cpp 
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
VFSSvc/*.cpp )

GAUDI_COMPONENT_LIBRARY(GaudiSvc ${GaudiSvc_srcs} LIBRARIES GaudiKernel ${ROOT_LIBRARIES} Hist RIO Tree ${CLHEP_LIBRARIES})
GAUDI_COMPONENT_LIBRARY(GaudiSvcTest ../tests/src/component/*.cpp LIBRARIES GaudiKernel ${Boost_regex})

#---This is needed by SET_RUNTIME_PATH() :-(
SET( GAUDI_LIBRARY_DIRS ${GAUDI_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE )

