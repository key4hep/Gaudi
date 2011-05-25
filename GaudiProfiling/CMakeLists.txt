############################################################################
# CMakeLists.txt file for building GaudiProfiling
############################################################################
require(GaudiKernel)

if (CMAKE_SYSTEM_NAME MATCHES Linux)

find_package(Boost COMPONENTS python)
find_package(PythonLibs)
find_package(unwind)

#---Libraries---------------------------------------------------------------
include_directories(src/component)
GAUDI_COMPONENT_LIBRARY(GaudiProfiling component/*.cpp
                        LIBRARIES GaudiKernel z
                        USE_HEADERS unwind)
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
