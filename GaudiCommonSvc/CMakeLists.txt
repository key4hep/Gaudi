require(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCommonSvc
                        *.cpp DataSvc/*.cpp HistogramSvc/*.cpp PersistencySvc/*.cpp
                        LINK_LIBRARIES GaudiKernel ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${ROOT_Hist_LIBRARY} ${ROOT_RIO_LIBRARY}
                        INCLUDE_DIRS Boost ROOT AIDA)
