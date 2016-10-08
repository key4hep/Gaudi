gaudi_subdir(GaudiCommonSvc v4r1)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT COMPONENTS Hist RIO)

# Hide some Boost/ROOT compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCommonSvc
                 src/*.cpp DataSvc/*.cpp HistogramSvc/*.cpp PersistencySvc/*.cpp
                 LINK_LIBRARIES GaudiKernel GaudiHive Boost ROOT
                 INCLUDE_DIRS Boost ROOT AIDA)

