gaudi_subdir(GaudiHive)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg GaudiCommonSvc GaudiCoreSvc)

find_package(Boost COMPONENTS system filesystem graph)
find_package(ROOT COMPONENTS Hist RIO)
find_package(TBB)
find_package(pyanalysis)
find_package(pytools)

set (LIBRT_NAME "rt")
if (${APPLE})
  set (LIBRT_NAME "")
endif (${APPLE})

# Hide some Boost/TBB compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiHive
                 *.cpp
                 LINK_LIBRARIES GaudiKernel Boost ROOT TBB GaudiAlgLib  ${LIBRT_NAME}
                 INCLUDE_DIRS Boost ROOT TBB)

gaudi_add_test(WhiteBoard
               FRAMEWORK options/testWhiteBoard.py
               TIMEOUT 120)

gaudi_add_test(WriteWhiteBoard
               FRAMEWORK options/WriteWhiteBoard.py
               TIMEOUT 120)

gaudi_add_test(ReadWhiteBoard
               FRAMEWORK options/ReadWhiteBoard.py
               DEPENDS WriteWhiteBoard
               TIMEOUT 120)

gaudi_add_test(ReadAndWriteWhiteBoard
               FRAMEWORK options/ReadAndWriteWhiteBoard.py
               DEPENDS WriteWhiteBoard
               TIMEOUT 120)

# Test the Scheduler
gaudi_add_test(ForwardScheduler
               FRAMEWORK options/ForwardSchedulerSimpleTest.py
               DEPENDS WhiteBoard
               TIMEOUT 120)

gaudi_add_test(ForwardSchedulerStall
               FRAMEWORK options/ForwardSchedulerStall.py
               DEPENDS ForwardScheduler,
               PASSREGEX "Stall detected"
               TIMEOUT 120)

gaudi_add_test(AvalancheSchedulerSimpleTest
               FRAMEWORK options/AvalancheSchedulerSimpleTest.py
               TIMEOUT 120)

gaudi_add_test(BrunelScenarioAvalancheScheduler
               FRAMEWORK options/BrunelScenarioAvalancheScheduler.py
               TIMEOUT 120)
set_property(TEST GaudiHive.BrunelScenarioAvalancheScheduler PROPERTY SKIP_RETURN_CODE 77)

gaudi_add_test(AtlasMCRecoScenario
               FRAMEWORK options/AtlasMCRecoScenario.py
               TIMEOUT 120)
set_property(TEST GaudiHive.AtlasMCRecoScenario PROPERTY SKIP_RETURN_CODE 77)

gaudi_add_test(AtlasMCRecoFullPrecedenceDump
               FRAMEWORK options/AtlasMCRecoFullPrecedenceDump.py
               TIMEOUT 120)
set_property(TEST GaudiHive.AtlasMCRecoFullPrecedenceDump PROPERTY SKIP_RETURN_CODE 77)

gaudi_add_test(SequentialSequencer
               FRAMEWORK options/SequentialAlgSequencerTest.py
               DEPENDS WhiteBoard
               TIMEOUT 120)

gaudi_add_test(ControlFlowBranching+CrossBranchDataFlow
               FRAMEWORK options/ControlFlowBranching+CrossBranchDataFlow.py
               TIMEOUT 120)

# BugFixes
gaudi_add_test(BugCFHEP114
               FRAMEWORK options/BugCFHEP114.py
               DEPENDS WhiteBoard
               TIMEOUT 120)

#---Installation-----------------------------------------------------------------------------
gaudi_install_python_modules()
