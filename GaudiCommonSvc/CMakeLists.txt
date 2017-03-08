gaudi_subdir(GaudiCommonSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem)
find_package(ROOT COMPONENTS Hist RIO)

# Decide whether to link against AIDA:
set( aida_lib )
set( histo_lib )
if( AIDA_FOUND )
  set( aida_lib AIDA )
  set( histo_lib GaudiCommonSvcLib )
endif()

# The list of sources to build into the module:
list(APPEND histo_svc_sources "")
set( module_sources src/*.cpp src/DataSvc/*.cpp
   src/PersistencySvc/*.cpp )
if( AIDA_FOUND )
   list( APPEND module_sources src/HistogramSvc/Factory.cpp
      src/HistogramPersistencySvc/*.cpp )
    list( APPEND histo_svc_sources src/HistogramSvc/HistogramSvc.cpp
                                   src/HistogramSvc/AIDA_visibility_hack.cpp
                                   src/HistogramSvc/H1D.cpp
                                   src/HistogramSvc/H2D.cpp
                                   src/HistogramSvc/H3D.cpp
                                   src/HistogramSvc/P1D.cpp
                                   src/HistogramSvc/P2D.cpp )
endif()

# Hide some Boost/ROOT compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
if( AIDA_FOUND )
  gaudi_add_library(GaudiCommonSvcLib
                    ${histo_svc_sources}
                    LINK_LIBRARIES GaudiKernel Boost ROOT
                    PUBLIC_HEADERS GaudiCommonSvc
                    INCLUDE_DIRS Boost ROOT ${aida_lib})
endif()

gaudi_add_module(GaudiCommonSvc
                 ${module_sources}
                 LINK_LIBRARIES GaudiKernel Boost ROOT ${histo_lib}
                 INCLUDE_DIRS Boost ROOT ${aida_lib})
