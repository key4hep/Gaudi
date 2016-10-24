gaudi_subdir(GaudiCoreSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread python)
find_package(TBB)
find_package(PythonLibs)
find_package(ROOT)

# Hide some Boost/ROOT/TBB compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCoreSvc
                 src/ApplicationMgr/*.cpp
                 src/EventSelector/*.cpp
                 src/IncidentSvc/*.cpp
                 src/JobOptionsSvc/*.cpp
                 src/MessageSvc/*.cpp
                 src/AlgExecStateSvc/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost TBB rt PythonLibs
                 INCLUDE_DIRS TBB PythonLibs)
