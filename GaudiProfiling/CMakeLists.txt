############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
if (CMAKE_SYSTEM_NAME MATCHES Linux)

GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiSvc)
#GAUDI_USE_PACKAGE(Python)
GAUDI_USE_PACKAGE(Boost)
#GAUDI_USE_PACKAGE(ROOT)
GAUDI_USE_PACKAGE(libunwind)

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_DIR})

#---Libraries---------------------------------------------------------------
include_directories(src/component)
GAUDI_COMPONENT_LIBRARY(GaudiProfiling component/*.cpp LIBRARIES GaudiKernel z)
GAUDI_PYTHON_MODULE(PyCPUFamily python/CPUFamily.cpp LIBRARIES ${Boost_python_LIBRARY} )

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(GaudiGenProfilingHtml app/pfm_gen_analysis.cpp LIBRARIES z GaudiKernel )

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiProfiling)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
endif()
