gaudi_subdir(GaudiSvc v21r3)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(ROOT COMPONENTS Hist RIO Tree Net)
find_package(CLHEP)

#---Libraries---------------------------------------------------------------
SET(GaudiSvc_srcs
    src/DetectorDataSvc/*.cpp
    src/NTupleSvc/*.cpp
    src/RndmGenSvc/*.cpp
    src/THistSvc/*.cpp
    src/FileMgr/*.cpp)

if(NOT GAUDI_ATLAS)
  gaudi_add_module(GaudiSvc ${GaudiSvc_srcs}
                   LINK_LIBRARIES GaudiKernel Boost ROOT CLHEP
                   INCLUDE_DIRS Boost ROOT AIDA CLHEP)
else()
  gaudi_add_module(GaudiSvc ${GaudiSvc_srcs}
                   LINK_LIBRARIES GaudiKernel Boost ROOT CLHEP
                   INCLUDE_DIRS Boost ROOT AIDA CLHEP
                   GENCONF_USER_MODULE GaudiSvc.ExtraModules)
endif()

if(GAUDI_BUILD_TESTS)
  gaudi_add_module(GaudiSvcTest tests/src/component/*.cpp LINK_LIBRARIES GaudiKernel ${Boost_REGEX_LIBRARY})
endif()

gaudi_install_python_modules()

#---Test-----------------------------------------------------------------------
gaudi_add_test(QMTest QMTEST)
