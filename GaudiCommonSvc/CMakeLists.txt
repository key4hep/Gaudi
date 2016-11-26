gaudi_subdir(GaudiCommonSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT COMPONENTS Hist RIO)

# Decide whether to link against AIDA:
set( aida_lib )
if( AIDA_FOUND )
   set( aida_lib AIDA )
endif()

# The list of sources to build into the module:
set( module_sources src/*.cpp src/DataSvc/*.cpp
   src/PersistencySvc/DataSvcFileEntriesTool.cpp
   src/PersistencySvc/OutputStreamAgent.cpp
   src/PersistencySvc/DetPersistencySvc.cpp
   src/PersistencySvc/PersistencySvc.cpp
   src/PersistencySvc/EvtCollectionStream.cpp
   src/PersistencySvc/RecordStream.cpp
   src/PersistencySvc/EvtPersistencySvc.cpp
   src/PersistencySvc/RunRecordStream.cpp
   src/PersistencySvc/SequentialOutputStream.cpp
   src/PersistencySvc/InputCopyStream.cpp
   src/PersistencySvc/TagCollectionStream.cpp
   src/PersistencySvc/OutputStream.cpp )
if( AIDA_FOUND )
   list( APPEND module_sources src/HistogramSvc/*.cpp
      src/PersistencySvc/HistogramPersistencySvc.cpp )
endif()

# Hide some Boost/ROOT compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiCommonSvc
                 ${module_sources}
                 LINK_LIBRARIES GaudiKernel Boost ROOT
                 INCLUDE_DIRS Boost ROOT ${aida_lib})
