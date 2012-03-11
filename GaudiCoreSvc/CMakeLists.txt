require(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread)

GAUDI_PACKAGE_VERSION_HEADER(GaudiCoreSvc v1r0 ${CMAKE_CURRENT_BINARY_DIR})

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCoreSvc
                        ApplicationMgr/*.cpp
                        EventSelector/*.cpp
                        IncidentSvc/*.cpp
                        JobOptionsSvc/*.cpp
                        MessageSvc/*.cpp
                        LINK_LIBRARIES GaudiKernel ${Boost_LIBRARIES})
add_dependencies(GaudiCoreSvc GaudiCoreSvcVersionHeader)
