require(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiCommonSvc
                        *.cpp DataSvc/*.cpp HistogramSvc/*.cpp PersistencySvc/*.cpp
                        LIBRARIES GaudiKernel ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${ROOT_Hist_LIBRARY} ${ROOT_RIO_LIBRARY}
                        USE_HEADERS Boost ROOT AIDA)
