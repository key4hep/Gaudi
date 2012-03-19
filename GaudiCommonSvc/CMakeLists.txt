require(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT COMPONENTS Hist RIO)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCommonSvc
                        *.cpp DataSvc/*.cpp HistogramSvc/*.cpp PersistencySvc/*.cpp
                        LINK_LIBRARIES GaudiKernel Boost ROOT
                        INCLUDE_DIRS Boost ROOT AIDA)
