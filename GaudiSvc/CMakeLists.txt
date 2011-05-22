############################################################################
# CMakeLists.txt file for building GaudiSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(CLHEP)

find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS regex)
include_directories(${Boost_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

find_package(CLHEP)
include_directories(${CLHEP_INCLUDE_DIRS})

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

GAUDI_COMPONENT_LIBRARY(GaudiSvc ${GaudiSvc_srcs} LIBRARIES GaudiKernel ${ROOT_LIBRARIES}
                        ${ROOT_Hist_LIBRARY} ${ROOT_RIO_LIBRARY} ${ROOT_Tree_LIBRARY} ${CLHEP_LIBRARIES} ${Boost_LIBRARIES})

if(BUILD_TESTS)
  GAUDI_COMPONENT_LIBRARY(GaudiSvcTest ../tests/src/component/*.cpp LIBRARIES GaudiKernel ${Boost_REGEX_LIBRARY})
endif()

#---Test-----------------------------------------------------------------------
GAUDI_QMTEST_TEST(gaudisvc.gaudisvctest)
