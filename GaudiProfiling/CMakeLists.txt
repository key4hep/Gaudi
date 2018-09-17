gaudi_subdir(GaudiProfiling)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg)

if (CMAKE_SYSTEM_NAME MATCHES Linux)

find_package(Boost COMPONENTS python REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(unwind)
find_package(gperftools)
find_package(IntelAmplifier)
find_package(ZLIB REQUIRED)

# Hide some Boost/libunwind compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
if( UNWIND_FOUND )
   include_directories( SYSTEM ${UNWIND_INCLUDE_DIRS} )
endif()

#---Components--------------------------------------------------------------
include_directories(src/component)

#-----------------------------------
# PerfMon profiler
#-----------------------------------
if(UNWIND_FOUND AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  gaudi_add_module(GaudiProfiling src/component/*.cpp
                   LINK_LIBRARIES GaudiKernel ZLIB
                   INCLUDE_DIRS unwind ZLIB)

  gaudi_add_python_module(PyCPUFamily src/python/CPUFamily.cpp
                          LINK_LIBRARIES ${Boost_PYTHON_LIBRARY}
                          INCLUDE_DIRS PythonLibs)

  gaudi_add_executable(GaudiGenProfilingHtml src/app/pfm_gen_analysis.cpp
                       LINK_LIBRARIES GaudiKernel ZLIB
                       INCLUDE_DIRS unwind ZLIB)
else()
  message(WARNING "The module GaudiProfiling is disabled (architecture ${CMAKE_SYSTEM_PROCESSOR}).")
endif()


#-----------------------------------
# Google PerfTools profiler
#-----------------------------------
if(GPERFTOOLS_FOUND)
  gaudi_add_module(GaudiGoogleProfiling src/component/google/*.cpp
                   LINK_LIBRARIES GaudiKernel GaudiAlgLib ZLIB
                   INCLUDE_DIRS ${GPERFTOOLS_INCLUDE_DIRS} ZLIB)

# Special handling of unresorlved symbols in GaudiGoogleProfiling.
# The profilers need to have libtcmalloc.so or libprofiler.so pre-loaded to
# work, so it's better if the symbols stay undefined in case somebody tries to
# use the profilers without preloading the libraries.
set(gprof_linker_flags)
foreach(undef_symbol IsHeapProfilerRunning
                     HeapProfilerStart HeapProfilerStop
                     HeapProfilerDump GetHeapProfile
                     ProfilerStart ProfilerStop
                     _ZN15HeapLeakCheckerC1EPKc
                     _ZN15HeapLeakChecker9DoNoLeaksENS_15ShouldSymbolizeE
                     _ZN15HeapLeakCheckerD1Ev)
  set(gprof_linker_flags "${gprof_linker_flags} -Wl,--defsym,${undef_symbol}=0")
endforeach()
set_target_properties(GaudiGoogleProfiling PROPERTIES LINK_FLAGS "${gprof_linker_flags}")

if(GPERFTOOLS_VERSION VERSION_LESS 2.0)
  set_property(TARGET GaudiGoogleProfiling
    APPEND PROPERTY COMPILE_DEFINITIONS TCMALLOC_OLD_GOOGLE_HEADERS)
endif()
else()
    message(STATUS "gperftools not found. Skipping GaudiGoogleProfiling")
endif()
#-----------------------------------
# Intel VTune profiler
#-----------------------------------
if(INTELAMPLIFIER_FOUND)
  gaudi_add_module(IntelProfiler src/component/intel/IntelProfilerAuditor.cpp
                   LINK_LIBRARIES ${INTELAMPLIFIER_LIBITTNOTIFY} GaudiKernel
                   INCLUDE_DIRS ${INTELAMPLIFIER_INCLUDE_DIRS})
  gaudi_add_module(GaudiIntelProfiling src/component/intel/IntelProfile.cpp
                   LINK_LIBRARIES ${INTELAMPLIFIER_LIBITTNOTIFY} GaudiKernel GaudiAlgLib
                   INCLUDE_DIRS ${INTELAMPLIFIER_INCLUDE_DIRS})
else()
  message(WARNING "VTune libraries not found, the module IntelProfiler is disabled")
endif()

#-----------------------------------
# Valgrind profiler
#-----------------------------------
gaudi_add_module(GaudiValgrindProfiling src/component/valgrind/*.cpp
                 LINK_LIBRARIES GaudiKernel GaudiAlgLib ZLIB
                 INCLUDE_DIRS tcmalloc ZLIB)


#-----------------------------------
# jemalloc
#-----------------------------------
gaudi_add_module(GaudiJemalloc src/component/jemalloc/*.cpp
                 LINK_LIBRARIES GaudiKernel GaudiAlgLib)

# Special handling of unresolved symbols in Jemmalloc.
# The profilers need to have libjemalloc.so o pre-loaded to
# work, so it's better if the symbols stay undefined in case somebody tries to
# use the profilers without preloading the libraries.
set(GaudiJemalloc_linker_flags)
foreach(undef_symbol mallctl)
  set(GaudiJemalloc_linker_flags "${GaudiJemalloc_linker_flags} -Wl,--defsym,${undef_symbol}=0")
endforeach()
set_target_properties(GaudiJemalloc PROPERTIES LINK_FLAGS "${GaudiJemalloc_linker_flags}")

find_library(jemalloc_lib NAMES jemalloc)
if(jemalloc_lib)
  message(STATUS "Found jemalloc: ${jemalloc_lib}")
  gaudi_add_test(jira.gaudi_1045
                 COMMAND gaudirun.py
                 ENVIRONMENT LD_PRELOAD=${jemalloc_lib})
  get_filename_component(jemalloc_libdir "${jemalloc_lib}" PATH)
  gaudi_env(PREPEND LD_LIBRARY_PATH "${jemalloc_libdir}")
else()
  message(STATUS "jemalloc not found, cannot test GAUDI-1045")
endif()

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()
endif()
