gaudi_subdir(GaudiAlg)

gaudi_depends_on_subdirs(GaudiUtils GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS regex)
find_package(ROOT COMPONENTS RIO)

# Decide whether to link against AIDA:
set( aida_lib )
if( AIDA_FOUND )
   set( aida_lib AIDA )
endif()

# The list of sources to build into the library:
set( lib_sources src/lib/EventCounter.cpp src/lib/IHistoTool.cpp
   src/lib/GaudiSequencer.cpp src/lib/ITupleTool.cpp
   src/lib/GaudiAlgorithm.cpp src/lib/GaudiTool.cpp
   src/lib/InterfaceVirtualDestructors.cpp src/lib/GaudiAtomicSequencer.cpp
   src/lib/Prescaler.cpp src/lib/GaudiCommon.icpp
   src/lib/GaudiCommonConstructors.cpp
   src/lib/Sequencer.cpp
   src/lib/GetAlg.cpp src/lib/Tuple.cpp src/lib/GaudiHistoID.cpp
   src/lib/TupleObj.cpp src/lib/IErrorTool.cpp )
if( AIDA_FOUND )
   list( APPEND lib_sources src/lib/Fill.cpp src/lib/Print.cpp
      src/lib/GaudiHistosConstructors.cpp src/lib/GaudiHistoAlg.cpp
      src/lib/GaudiHistoTool.cpp src/lib/GetAlgs.cpp
      src/lib/GaudiTupleAlg.cpp src/lib/GaudiTupleTool.cpp
      src/lib/GaudiTuplesConstructors.cpp )
endif()

# The list of sources to build into the module:
set( module_sources src/components/ErrorTool.cpp
   src/components/TimingAuditor.cpp src/components/EventNodeKiller.cpp
   src/components/GaudiAlg_entries.cpp src/components/TimerForSequencer.cpp )
if( AIDA_FOUND )
   list( APPEND module_sources src/components/HistTool.cpp
      src/components/TupleTool.cpp src/components/SequencerTimerTool.cpp )
endif()

# Hide some Boost/ROOT compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiAlgLib ${lib_sources}
                  LINK_LIBRARIES GaudiUtilsLib Boost
                  INCLUDE_DIRS Boost ${aida_lib}
                  PUBLIC_HEADERS GaudiAlg)
gaudi_add_module(GaudiAlg ${module_sources}
                 LINK_LIBRARIES GaudiAlgLib GaudiKernel ROOT
                 INCLUDE_DIRS Boost ${aida_lib})
