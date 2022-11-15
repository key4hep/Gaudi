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
# GaudiProfiling subdirectory

# If we do not build for Linux, give up on this subdirectory:
if(NOT CMAKE_SYSTEM_NAME MATCHES Linux)
    message(AUTHOR_WARNING "GaudiProfiling is only supported on Linux")
    return()
endif()

gaudi_add_python_module(PyCPUFamily
                        SOURCES src/python/CPUFamily.cpp
                        LINK Python::Python
                             Boost::python)

#-----------------------------------
# PerfMon profiler
#-----------------------------------
if(GAUDI_USE_UNWIND AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  gaudi_add_module(GaudiProfiling
                   SOURCES src/component/IgHook_IgHookTrace.cpp
                           src/component/PerfMonAuditor.cpp
                   LINK GaudiKernel
                        ZLIB::ZLIB
                        unwind::unwind)
  target_include_directories(GaudiProfiling PRIVATE src/component)

  gaudi_add_executable(GaudiGenProfilingHtml
                       SOURCES src/app/pfm_gen_analysis.cpp
                       LINK GaudiKernel
                            ZLIB::ZLIB
                            unwind::unwind)
else()
  message(STATUS "GaudiProfiling: the module PerfMon profiler is disabled")
endif()

#-----------------------------------
# Google PerfTools profiler
#-----------------------------------
if(GAUDI_USE_GPERFTOOLS)
  # Note we do not link against libtcmalloc.so or libprofiler.so as they have to be
  # preloaded (see next comment)
  gaudi_add_module(GaudiGoogleProfiling
                   SOURCES src/component/google/GoogleAuditor.cpp
                   LINK GaudiKernel
                        GaudiAlgLib
                        ZLIB::ZLIB)
  # but we still need to add the include directories to the search path
  target_include_directories(GaudiGoogleProfiling
    PRIVATE $<TARGET_PROPERTY:PkgConfig::gperftools,INTERFACE_INCLUDE_DIRECTORIES>)

  # Special handling of unresolved symbols in GaudiGoogleProfiling.
  # The profilers need to have libtcmalloc.so or libprofiler.so pre-loaded to
  # work, so it's better if the symbols stay undefined in case somebody tries to
  # use the profilers without preloading the libraries.
  set(gprof_linker_flags)
  foreach(undef_symbol IN ITEMS IsHeapProfilerRunning
                                HeapProfilerStart HeapProfilerStop
                                HeapProfilerDump GetHeapProfile
                                ProfilerStart ProfilerStop
                                _ZN15HeapLeakCheckerC1EPKc
                                _ZN15HeapLeakChecker9DoNoLeaksENS_15ShouldSymbolizeE
                                _ZN15HeapLeakCheckerD1Ev)
    list(APPEND gprof_linker_flags "-Wl,--defsym,${undef_symbol}=0")
  endforeach()
  set_target_properties(GaudiGoogleProfiling PROPERTIES LINK_OPTIONS "${gprof_linker_flags}")
else()
    message(STATUS "GaudiProfiling: the module Google PerfTools profiler is disabled")
endif()

#-----------------------------------
# Intel VTune profiler
#-----------------------------------
if(GAUDI_USE_INTELAMPLIFIER)
  gaudi_add_module(IntelProfiler
                   SOURCES src/component/intel/IntelProfilerAuditor.cpp
                   LINK GaudiKernel
                        IntelAmplifier::libittnotify)
  gaudi_add_module(GaudiIntelProfiling
                   SOURCES src/component/intel/IntelProfile.cpp
                   LINK GaudiKernel
                        GaudiAlgLib
                        IntelAmplifier::libittnotify)
else()
  message(STATUS "GaudiProfiling: the module Intel VTune profiler is disabled")
endif()

#-----------------------------------
# Valgrind profiler
#-----------------------------------
gaudi_add_module(GaudiValgrindProfiling
                 SOURCES src/component/valgrind/CallgrindProfile.cpp
                 LINK GaudiKernel
                      GaudiAlgLib
                      ZLIB::ZLIB)

#-----------------------------------
# jemalloc
#-----------------------------------
gaudi_add_module(GaudiJemalloc
                 SOURCES src/component/jemalloc/JemallocProfile.cpp
                         src/component/jemalloc/JemallocProfileSvc.cpp
                 LINK GaudiKernel
                      GaudiAlgLib)

# Special handling of unresolved symbols in Jemmalloc.
# The profilers need to have libjemalloc.so pre-loaded to
# work, so it's better if the symbols stay undefined in case somebody tries to
# use the profilers without preloading the libraries.
set_target_properties(GaudiJemalloc PROPERTIES LINK_OPTIONS "-Wl,--defsym,mallctl=0")

if(GAUDI_USE_JEMALLOC)
  add_test(NAME GaudiProfiling.jira.gaudi_1045
           COMMAND run ${CMAKE_COMMAND} -E env LD_PRELOAD=$<TARGET_PROPERTY:jemalloc::jemalloc,LOCATION> gaudirun.py)
endif()

# Register the header only library (install include/)
gaudi_add_header_only_library(GaudiProfilingHeaders)
# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)
