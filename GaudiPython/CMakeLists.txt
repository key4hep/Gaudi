############################################################################
# CMakeLists.txt file for building GaudiPython
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(GaudiAlg)
#G AUDI_USE_PACKAGE(GaudiUtils)
#G AUDI_USE_PACKAGE(AIDA)
#G AUDI_USE_PACKAGE(CLHEP)
#G AUDI_USE_PACKAGE(Python)


find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

find_package(CLHEP)
include_directories(${CLHEP_INCLUDE_DIRS})

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

REFLEX_BUILD_DICTIONARY(GaudiPython dict/kernel.h  dict/selection_kernel.xml LIBRARIES GaudiPythonLib GaudiUtilsLib)

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiPythonLib Lib/*.cpp LIBRARIES GaudiAlgLib ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES})
GAUDI_COMPONENT_LIBRARY(GaudiPython Services/*.cpp LIBRARIES GaudiPythonLib GaudiUtilsLib)

if(BUILD_TESTS)
  GAUDI_LINKER_LIBRARY(GPyTest Test/*.cpp LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES})
  REFLEX_BUILD_DICTIONARY(GPyTest src/Test/test.h src/Test/test_selection.xml LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES})
  include_directories(src/Test)
endif()

#---Dictionaries------------------------------------------------------------

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiPython)
GAUDI_INSTALL_PYTHON_MODULES()

