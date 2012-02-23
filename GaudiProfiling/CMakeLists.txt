############################################################################
# CMakeLists.txt file for building GaudiProfiling
############################################################################
require(GaudiKernel GaudiAlg)

if (CMAKE_SYSTEM_NAME MATCHES Linux)

find_package(Boost COMPONENTS python)
find_package(PythonLibs)
find_package(unwind)
find_package(tcmalloc)

#---Libraries---------------------------------------------------------------
include_directories(src/component)
GAUDI_COMPONENT_LIBRARY(GaudiProfiling component/*.cpp
                        LIBRARIES GaudiKernel z
                        USE_HEADERS unwind)
GAUDI_COMPONENT_LIBRARY(GaudiGoogleProfiling component/google/*.cpp
                        LIBRARIES GaudiKernel GaudiAlgLib z
                        USE_HEADERS tcmalloc)
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

GAUDI_PYTHON_MODULE(PyCPUFamily python/CPUFamily.cpp
                    LIBRARIES ${Boost_PYTHON_LIBRARY}
                    USE_HEADERS PythonLibs)

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(GaudiGenProfilingHtml app/pfm_gen_analysis.cpp
                 LIBRARIES z GaudiKernel)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiProfiling)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
endif()
