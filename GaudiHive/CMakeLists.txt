#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# GaudiHive subdirectory

# Build the plugin
gaudi_add_module(GaudiHive
                 SOURCES src/AlgResourcePool.cpp
                         src/AlgsExecutionStates.cpp
                         src/AvalancheSchedulerSvc.cpp
                         src/ContextEventCounter.cpp
                         src/CPUCruncher.cpp
                         src/FetchDataFromFile.cpp
                         src/FetchLeavesFromFile.cpp
                         src/HiveDataBroker.cpp
                         src/HiveNumbers.cpp
                         src/HiveReadAlgorithm.cpp
                         src/HiveSlimEventLoopMgr.cpp
                         src/HiveTestAlgorithm.cpp
                         src/HiveWhiteBoard.cpp
                         src/PrecedenceSvc.cpp
                         src/PRGraph/PrecedenceRulesGraph.cpp
                         src/PRGraph/Visitors/Promoters.cpp
                         src/PRGraph/Visitors/Rankers.cpp
                         src/PRGraph/Visitors/Validators.cpp
                         src/ThreadInitTask.cpp
                         src/ThreadPoolSvc.cpp
                         src/TimelineSvc.cpp
                         src/ViewTester.cpp
                 LINK GaudiKernel
                      Gaudi::Functional
                      Boost::headers
                      Boost::system
                      Boost::filesystem
                      Boost::fiber
                      Boost::graph
                      ROOT::Hist
                      ROOT::RIO
                      TBB::tbb
                      Rangev3::rangev3
                      fmt::fmt
                      ${rt_LIBRARY}
                      ${GAUDI_ATOMIC_LIBS})

if (GAUDI_USE_CUDA)
  target_sources(GaudiHive PRIVATE
                         src/GPUCruncher.cpp
                         src/GPUCruncher.cu )
endif()
# Build executable
gaudi_add_executable(makeThesis
                     SOURCES src/bin/makeThesis.cpp)
gaudi_add_executable(concurrentRun
                     SOURCES src/bin/concurrentRun.cpp
                     LINK GaudiKernel
                          Boost::headers
                          Boost::program_options)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)

gaudi_add_pytest(tests/pytest)
