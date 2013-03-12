gaudi_subdir(GaudiProfiling v1r8)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg)

if (CMAKE_SYSTEM_NAME MATCHES Linux)

find_package(Boost COMPONENTS python)
find_package(PythonLibs)
find_package(unwind)
find_package(tcmalloc)

#---------------------------------------------------------------------------
# Hack the VTune detection
if(NOT VTune_FOUND)
  set(VTune_version 2011)
  set(VTune_home /afs/cern.ch/sw/IntelSoftware/linux/${CMAKE_HOST_SYSTEM_PROCESSOR}/xe${VTune_version}/vtune_amplifier_xe_${VTune_version})
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(VTune_LIBRARY_DIR_HINT ${VTune_home}/lib64)
  else()
    set(VTune_LIBRARY_DIR_HINT ${VTune_home}/lib32)
  endif()
  set(VTune_INCLUDE_DIR_HINT ${VTune_home}/include)

  find_library(VTune_LIBRARIES ittnotify HINTS ${VTune_LIBRARY_DIR_HINT})
  find_path(VTune_INCLUDE_DIRS ittnotify.h HINTS ${VTune_INCLUDE_DIR_HINT})

  if(VTune_LIBRARIES AND VTune_INCLUDE_DIRS)
    set(VTune_FOUND ${VTune_LIBRARIES} CACHE INTERNAL "")
    message(STATUS "Found VTune: ${VTune_LIBRARIES}")
  endif()
  mark_as_advanced(VTune_INCLUDE_DIRS VTune_LIBRARIES)
endif()
#---------------------------------------------------------------------------

#---Components--------------------------------------------------------------
include_directories(src/component)

#-----------------------------------
# PerfMon profiler
#-----------------------------------
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  gaudi_add_module(GaudiProfiling src/component/*.cpp
                   LINK_LIBRARIES GaudiKernel z
                   INCLUDE_DIRS unwind)

  gaudi_add_python_module(PyCPUFamily src/python/CPUFamily.cpp
                          LINK_LIBRARIES ${Boost_PYTHON_LIBRARY}
                          INCLUDE_DIRS PythonLibs)

  gaudi_add_executable(GaudiGenProfilingHtml src/app/pfm_gen_analysis.cpp
                       LINK_LIBRARIES z GaudiKernel)
else()
  message(WARNING "The module GaudiProfiling is disabled (unsupported architecture ${CMAKE_SYSTEM_PROCESSOR}).")
endif()


#-----------------------------------
# Google PerfTools profiler
#-----------------------------------
gaudi_add_module(GaudiGoogleProfiling src/component/google/*.cpp
                 LINK_LIBRARIES GaudiKernel GaudiAlgLib z
                 INCLUDE_DIRS tcmalloc)

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


#-----------------------------------
# Intel VTune profiler
#-----------------------------------
if(VTune_FOUND)
  gaudi_add_module(IntelProfiler src/component/intel/*.cpp
                   LINK_LIBRARIES ${VTune_LIBRARIES} GaudiKernel
                   INCLUDE_DIRS ${VTune_INCLUDE_DIRS})
else()
  message(WARNING "VTune libraries not found, the module IntelProfiler is disabled")
endif()

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()
endif()
