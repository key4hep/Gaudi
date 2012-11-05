gaudi_subdir(GaudiCoreSvc v1r2)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread)

# Boost Spirit (1.48) gives a compilation error (extra ';') when -pedantic is used
string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCoreSvc
                 src/ApplicationMgr/*.cpp
                 src/EventSelector/*.cpp
                 src/IncidentSvc/*.cpp
                 src/JobOptionsSvc/*.cpp
                 src/MessageSvc/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost)
