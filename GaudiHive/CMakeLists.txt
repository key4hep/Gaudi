gaudi_subdir(GaudiHive v1r0)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg GaudiCommonSvc GaudiCoreSvc)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem graph)
find_package(ROOT COMPONENTS Hist RIO)
find_package(TBB)
find_package(pyanalysis)

set (LIBRT_NAME "rt")
if (${APPLE})
  set (LIBRT_NAME "")
endif (${APPLE})


#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiHive
                        *.cpp
                        LINK_LIBRARIES GaudiKernel Boost ROOT TBB GaudiAlgLib  ${LIBRT_NAME}
                        INCLUDE_DIRS Boost ROOT AIDA TBB)

gaudi_add_test(WhiteBoard
               FRAMEWORK options/testWhiteBoard.py)

gaudi_add_test(CPUCruncher
               FRAMEWORK options/CPUCruncher.py)

gaudi_add_test(WriteWhiteBoard
               FRAMEWORK options/WriteWhiteBoard.py )

gaudi_add_test(ReadWhiteBoard
               FRAMEWORK options/ReadWhiteBoard.py
               DEPENDS WriteWhiteBoard)

gaudi_add_test(ReadAndWriteWhiteBoard
               FRAMEWORK options/ReadAndWriteWhiteBoard.py
               DEPENDS WriteWhiteBoard)

# Test the Scheduler

gaudi_add_test(ForwardScheduler
               FRAMEWORK options/ForwardSchedulerSimpleTest.py
               DEPENDS WhiteBoard)

gaudi_add_test(ForwardSchedulerStall
               FRAMEWORK options/ForwardSchedulerStall.py
               DEPENDS ForwardScheduler,
               PASSREGEX "Stall detected")

gaudi_add_test(SequentialSchedulerSimpleTest
               FRAMEWORK options/SequentialSchedulerSimpleTest.py)

gaudi_add_test(GraphSchedulerSimpleTest
               FRAMEWORK options/GraphSchedulerSimpleTest.py)

# Test the AtomicSequencer
gaudi_add_test(AtomicSequencer
               FRAMEWORK options/AtomicAlgSequencerTest.py
               DEPENDS WhiteBoard)

# BugFixes

gaudi_add_test(BugCFHEP114
               FRAMEWORK options/BugCFHEP114.py
               DEPENDS WhiteBoard)
