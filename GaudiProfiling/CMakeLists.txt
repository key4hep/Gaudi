############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
if (CMAKE_SYSTEM_NAME MATCHES Linux)

#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(GaudiSvc)
##G AUDI_USE_PACKAGE(Python)
#G AUDI_USE_PACKAGE(Boost)
##G AUDI_USE_PACKAGE(ROOT)
#G AUDI_USE_PACKAGE(libunwind)

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

find_package(Boost COMPONENTS python)
include_directories(${Boost_INCLUDE_DIRS})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

find_package(unwind)
include_directories(${unwind_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
include_directories(src/component)
GAUDI_COMPONENT_LIBRARY(GaudiProfiling component/*.cpp LIBRARIES GaudiKernel z)
GAUDI_PYTHON_MODULE(PyCPUFamily python/CPUFamily.cpp LIBRARIES ${Boost_PYTHON_LIBRARY} )

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(GaudiGenProfilingHtml app/pfm_gen_analysis.cpp LIBRARIES z GaudiKernel )

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiProfiling)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
endif()
