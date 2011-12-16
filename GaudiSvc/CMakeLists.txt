############################################################################
# CMakeLists.txt file for building GaudiSvc
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

find_package(AIDA)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(ROOT)
find_package(CLHEP)

#---Libraries---------------------------------------------------------------
SET(GaudiSvc_srcs
DetectorDataSvc/*.cpp
NTupleSvc/*.cpp
RndmGenSvc/*.cpp
THistSvc/*.cpp)

GAUDI_COMPONENT_LIBRARY(GaudiSvc ${GaudiSvc_srcs}
                        LIBRARIES GaudiKernel ${ROOT_LIBRARIES} ${ROOT_Hist_LIBRARY}
                                  ${ROOT_RIO_LIBRARY} ${ROOT_Tree_LIBRARY} ${CLHEP_LIBRARIES}
                                  ${Boost_LIBRARIES}
                        USE_HEADERS Boost ROOT AIDA CLHEP)

if(BUILD_TESTS)
  GAUDI_COMPONENT_LIBRARY(GaudiSvcTest ../tests/src/component/*.cpp LIBRARIES GaudiKernel ${Boost_REGEX_LIBRARY})
endif()

#---Test-----------------------------------------------------------------------
GAUDI_QMTEST_TEST(gaudisvc.gaudisvctest)
