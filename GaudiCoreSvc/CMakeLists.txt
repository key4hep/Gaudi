gaudi_subdir(GaudiCoreSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread python27 REQUIRED)
find_package(TBB REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(ROOT REQUIRED)

# Hide some Boost/ROOT/TBB compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})

# Only link against the rt library on linux:
set( extralibs )
if( UNIX AND NOT APPLE )
   set( extralibs rt )
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCoreSvc
                 src/ApplicationMgr/*.cpp
                 src/EventSelector/*.cpp
                 src/IncidentSvc/*.cpp
                 src/JobOptionsSvc/*.cpp
                 src/MessageSvc/*.cpp
                 src/AlgExecStateSvc/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost TBB PythonLibs ${extralibs}
                 INCLUDE_DIRS TBB PythonLibs)
