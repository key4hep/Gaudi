gaudi_subdir(GaudiCommonSvc v3r4)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT COMPONENTS Hist RIO)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCommonSvc
                 src/*.cpp DataSvc/*.cpp HistogramSvc/*.cpp PersistencySvc/*.cpp
                 LINK_LIBRARIES GaudiKernel GaudiHive Boost ROOT
                 INCLUDE_DIRS Boost ROOT AIDA)

