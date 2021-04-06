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
# GaudiCoreSvc subdirectory

# Build the plugin library
gaudi_add_module(GaudiCoreSvc
                 SOURCES
                   src/AlgExecStateSvc/AlgExecStateSvc.cpp
                   src/ApplicationMgr/AlgorithmManager.cpp
                   src/ApplicationMgr/ApplicationMgr.cpp
                   src/ApplicationMgr/AppMgrRunable.cpp
                   src/ApplicationMgr/DLLClassManager.cpp
                   src/ApplicationMgr/EventLoopMgr.cpp
                   src/ApplicationMgr/MinimalEventLoopMgr.cpp
                   src/ApplicationMgr/ServiceManager.cpp
                   src/ApplicationMgr/ToolSvc.cpp
                   src/EventSelector/DataStreamToolFactory.cpp
                   src/EventSelector/EventCollectionSelector.cpp
                   src/EventSelector/EventSelector.cpp
                   src/IncidentSvc/DataOnDemandSvc.cpp
                   src/IncidentSvc/DODBasicMapper.cpp
                   src/IncidentSvc/IncidentProcAlg.cpp
                   src/IncidentSvc/IncidentSvc.cpp
                   src/JobOptionsSvc/Analyzer.cpp
                   src/JobOptionsSvc/Catalog.cpp
                   src/JobOptionsSvc/IncludedFiles.cpp
                   src/JobOptionsSvc/JobOptionsSvc.cpp
                   src/JobOptionsSvc/Message.cpp
                   src/JobOptionsSvc/Node.cpp
                   src/JobOptionsSvc/Parser.cpp
                   src/JobOptionsSvc/Position.cpp
                   src/JobOptionsSvc/Property.cpp
                   src/JobOptionsSvc/PropertyName.cpp
                   src/JobOptionsSvc/PropertyValue.cpp
                   src/JobOptionsSvc/PythonConfig.cpp
                   src/JobOptionsSvc/Units.cpp
                   src/JobOptionsSvc/Utils.cpp
                   src/MessageSvc/InertMessageSvc.cpp
                   src/MessageSvc/MessageSvc.cpp
                   src/MessageSvc/MessageSvcSink.cpp
                 LINK
                   GaudiKernel
                   Python::Python
                   Boost::headers
                   Boost::filesystem
                   Boost::python
                   Boost::regex
                   Boost::system
                   Boost::thread
                   TBB::tbb
                   ${rt_LIBRARY} # UNIX only
                   ${GAUDI_ATOMIC_LIBS})

foreach(name IN ITEMS base binding bwcompat custom)
    gaudi_add_executable(test_JOS_${name} SOURCES tests/src/test_JOS/${name}.cpp
                         LINK GaudiKernel Boost::unit_test_framework TEST)
endforeach()

gaudi_add_executable(JOS_benchmark SOURCES tests/src/test_JOS/benchmark.cpp
                     LINK GaudiKernel)
