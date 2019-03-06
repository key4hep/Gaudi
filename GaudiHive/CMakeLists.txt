gaudi_subdir(GaudiHive)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg GaudiCommonSvc GaudiCoreSvc)

find_package(Boost COMPONENTS system filesystem graph REQUIRED)
find_package(ROOT COMPONENTS Hist RIO REQUIRED)
find_package(TBB REQUIRED)
find_package(pyanalysis)
find_package(pytools)

set (LIBRT_NAME "rt")
if (${APPLE})
  set (LIBRT_NAME "")
endif (${APPLE})

# Hide some Boost/TBB compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
set(GaudiHive_srcs
    src/*.cpp
    PRGraph/*.cpp
    PRGraph/Visitors/*.cpp)

gaudi_add_module(GaudiHive ${GaudiHive_srcs}
                 LINK_LIBRARIES GaudiKernel Boost ROOT TBB GaudiAlgLib  ${LIBRT_NAME}
                 INCLUDE_DIRS Boost ROOT TBB)


gaudi_add_test(QMTest QMTEST)

#---Installation-----------------------------------------------------------------------------
gaudi_install_python_modules()
