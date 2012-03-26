require(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCoreSvc
                 ApplicationMgr/*.cpp
                 EventSelector/*.cpp
                 IncidentSvc/*.cpp
                 JobOptionsSvc/*.cpp
                 MessageSvc/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost)

# Generate the version header for the package.
execute_process(COMMAND
                  ${versheader_cmd}
                    GaudiCoreSvc v1r0 ${CMAKE_CURRENT_BINARY_DIR}/GaudiCoreSvcVersion.h)
