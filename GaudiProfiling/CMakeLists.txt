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
gaudi_add_module(GaudiProfiling component/*.cpp
                 LINK_LIBRARIES GaudiKernel z
                 INCLUDE_DIRS unwind)
gaudi_add_module(GaudiGoogleProfiling component/google/*.cpp
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

gaudi_add_python_module(PyCPUFamily python/CPUFamily.cpp
                    LINK_LIBRARIES ${Boost_PYTHON_LIBRARY}
                    INCLUDE_DIRS PythonLibs)

#---Executables-------------------------------------------------------------
gaudi_add_executable(GaudiGenProfilingHtml app/pfm_gen_analysis.cpp
                     LINK_LIBRARIES z GaudiKernel)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()
endif()
