gaudi_subdir(GaudiSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost REQUIRED COMPONENTS regex)
find_package(ROOT REQUIRED COMPONENTS Hist RIO Tree Net Matrix Thread MathCore)
find_package(TBB REQUIRED)
find_package(CLHEP)

# Decide whether to link against CLHEP:
set( clhep_lib )
if( CLHEP_FOUND )
   set( clhep_lib CLHEP )
endif()

# Hide some Boost/ROOT/CLHEP compile time warnings
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS} )
if( CLHEP_FOUND )
   include_directories( SYSTEM ${CLHEP_INCLUDE_DIRS} )
endif()

#---Libraries---------------------------------------------------------------
set( GaudiSvc_srcs
   src/DetectorDataSvc/*.cpp
   src/NTupleSvc/*.cpp
   src/THistSvc/*.cpp
   src/FileMgr/*.cpp
   src/CPUCrunchSvc/*.cpp
   src/MetaDataSvc/*.cpp )
if( CLHEP_FOUND )
   list( APPEND GaudiSvc_srcs
      src/RndmGenSvc/*.cpp )
endif()

if(NOT GAUDI_ATLAS)
  gaudi_add_module(GaudiSvc ${GaudiSvc_srcs}
                   LINK_LIBRARIES GaudiKernel Boost ROOT ${clhep_lib}
                   INCLUDE_DIRS Boost ROOT ${clhep_lib})
else()
  gaudi_add_module(GaudiSvc ${GaudiSvc_srcs}
                   LINK_LIBRARIES GaudiKernel Boost ROOT ${clhep_lib}
                   INCLUDE_DIRS Boost ROOT ${clhep_lib}
                   GENCONF_USER_MODULE GaudiSvc.ExtraModules)
endif()

gaudi_install_python_modules()
gaudi_install_scripts()

#---Test-----------------------------------------------------------------------
gaudi_add_test(QMTest QMTEST)
