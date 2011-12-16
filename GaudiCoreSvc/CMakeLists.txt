require(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiCoreSvc
                        ApplicationMgr/*.cpp
                        EventSelector/*.cpp
                        IncidentSvc/*.cpp
                        JobOptionsSvc/*.cpp
                        MessageSvc/*.cpp
                        LIBRARIES GaudiKernel ${Boost_LIBRARIES})
