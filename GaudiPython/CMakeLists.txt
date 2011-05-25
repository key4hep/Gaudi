############################################################################
# CMakeLists.txt file for building GaudiPython
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiAlg GaudiUtils GaudiKernel)

find_package(CLHEP)
find_package(PythonLibs)

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiPythonLib Lib/*.cpp
                     LIBRARIES GaudiAlgLib ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES}
                     USE_HEADERS CLHEP PythonLibs)
GAUDI_COMPONENT_LIBRARY(GaudiPython Services/*.cpp LIBRARIES GaudiPythonLib GaudiUtilsLib)

REFLEX_BUILD_DICTIONARY(GaudiPython dict/kernel.h  dict/selection_kernel.xml LIBRARIES GaudiPythonLib GaudiUtilsLib)

if(BUILD_TESTS)
  GAUDI_LINKER_LIBRARY(GPyTest Test/*.cpp
                       LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES})
  REFLEX_BUILD_DICTIONARY(GPyTest src/Test/test.h src/Test/test_selection.xml LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES})
  include_directories(src/Test)
endif()

#---Dictionaries------------------------------------------------------------

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiPython)
GAUDI_INSTALL_PYTHON_MODULES()

