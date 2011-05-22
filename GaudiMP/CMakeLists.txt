############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(GaudiPython)
#G AUDI_USE_PACKAGE(ROOT)

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiMP Lib/*.cpp LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${ROOT_LIBRARIES})

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiMP dict/gaudimp_dict.h dict/selection.xml LIBRARIES GaudiMP)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiMP)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
