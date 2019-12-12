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
gaudi_subdir(GaudiCommonSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(ROOT COMPONENTS Hist RIO REQUIRED)
find_package(TBB REQUIRED)

# Hide some Boost/ROOT/TBB compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})

# Decide whether to link against AIDA:
set( aida_lib )
set( histo_lib )
if( AIDA_FOUND )
  set( aida_lib AIDA )
  set( histo_lib GaudiCommonSvcLib )
endif()

# The list of sources to build into the module:
set(histo_svc_sources)
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


#---Libraries---------------------------------------------------------------
if( AIDA_FOUND )
  gaudi_add_library(GaudiCommonSvcLib
                    ${histo_svc_sources}
                    LINK_LIBRARIES GaudiKernel Boost ROOT TBB
                    PUBLIC_HEADERS GaudiCommonSvc
                    INCLUDE_DIRS Boost ROOT TBB ${aida_lib})
endif()

gaudi_add_module(GaudiCommonSvc
                 ${module_sources}
                 LINK_LIBRARIES GaudiKernel Boost ROOT TBB ${histo_lib}
                 INCLUDE_DIRS Boost ROOT TBB ${aida_lib})
