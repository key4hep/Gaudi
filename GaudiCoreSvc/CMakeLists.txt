#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
gaudi_subdir(GaudiCoreSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS system filesystem regex thread python${boost_python_version} REQUIRED)
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

foreach(name IN ITEMS base binding bwcompat custom)
    gaudi_add_unit_test(test_JOS_${name} tests/src/test_JOS/${name}.cpp
                        LINK_LIBRARIES GaudiKernel TYPE Boost)
   add_dependencies(test_JOS_${name} GaudiCoreSvc)
endforeach()

gaudi_add_executable(JOS_benchmark tests/src/test_JOS/benchmark.cpp
                     LINK_LIBRARIES GaudiKernel)
add_dependencies(JOS_benchmark GaudiCoreSvc)
