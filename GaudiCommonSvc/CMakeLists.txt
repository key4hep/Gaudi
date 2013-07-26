gaudi_subdir(GaudiCommonSvc v1r6)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT COMPONENTS Hist RIO)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCommonSvc
                 src/*.cpp src/DataSvc/*.cpp src/HistogramSvc/*.cpp src/PersistencySvc/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost ROOT
                 INCLUDE_DIRS Boost ROOT AIDA)
