gaudi_subdir(GaudiCoreSvc v1r2)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread)
find_package(TBB)

# FIXME: this works only when using the HEPTools toolchain.
if(LCG_HOST_COMP STREQUAL gcc AND LCG_HOST_COMPVERS STREQUAL 43)
  # Boost Spirit (1.48) gives a compilation error (extra ';') when -pedantic is
  # used (gcc 4.3 only, for more recent version it's just a warning).
  string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCoreSvc
                 src/ApplicationMgr/*.cpp
                 src/EventSelector/*.cpp
                 src/IncidentSvc/*.cpp
                 src/JobOptionsSvc/*.cpp
                 src/MessageSvc/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost TBB GaudiCoreSvcHive
                 INCLUDE_DIRS TBB)

gaudi_add_library(GaudiCoreSvcHive Hive/*.cpp
                  LINK_LIBRARIES GaudiKernel Boost TBB
                  INCLUDE_DIRS TBB
                  NO_PUBLIC_HEADERS)

