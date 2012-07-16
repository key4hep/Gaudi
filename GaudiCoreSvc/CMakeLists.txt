gaudi_subdir(GaudiCoreSvc v1r2)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread)
find_package(TBB)

# Boost Spirit (1.48) gives a compilation error (extra ';') when -pedantic is used
string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCoreSvc
                 ApplicationMgr/*.cpp
                 EventSelector/*.cpp
                 IncidentSvc/*.cpp
                 JobOptionsSvc/*.cpp
                 MessageSvc/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost TBB
                 INCLUDE_DIRS TBB)
