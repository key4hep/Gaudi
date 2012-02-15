require(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread)

GAUDI_PACKAGE_VERSION_HEADER(GaudiCoreSvc v1r0 ${CMAKE_CURRENT_BINARY_DIR})

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiCoreSvc
                        ApplicationMgr/*.cpp
                        EventSelector/*.cpp
                        IncidentSvc/*.cpp
                        JobOptionsSvc/*.cpp
                        MessageSvc/*.cpp
                        LIBRARIES GaudiKernel ${Boost_LIBRARIES})
add_dependencies(GaudiCoreSvc GaudiCoreSvcVersionHeader)
