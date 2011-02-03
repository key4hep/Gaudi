############################################################################
# CMakeLists.txt file for building GaudiPython
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiAlg)
GAUDI_USE_PACKAGE(GaudiUtils)
GAUDI_USE_PACKAGE(AIDA)
GAUDI_USE_PACKAGE(CLHEP)
GAUDI_USE_PACKAGE(Python)

REFLEX_BUILD_DICTIONARY(GaudiPython dict/kernel.h  dict/selection_kernel.xml LIBRARIES GaudiPythonLib GaudiUtilsLib)

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiPythonLib Lib/*.cpp LIBRARIES GaudiAlgLib ${Python_LIBRARIES} ${CLHEP_LIBRARIES})
GAUDI_COMPONENT_LIBRARY(GaudiPython Services/*.cpp LIBRARIES GaudiPythonLib GaudiUtilsLib)

if(BUILD_TESTS)
  GAUDI_LINKER_LIBRARY(GPyTest Test/*.cpp LIBRARIES GaudiKernel ${Python_LIBRARIES} ${CLHEP_LIBRARIES})
  REFLEX_BUILD_DICTIONARY(GPyTest src/Test/test.h src/Test/test_selection.xml LIBRARIES GaudiKernel ${Python_LIBRARIES} ${CLHEP_LIBRARIES})
  include_directories(src/Test)
endif()

#---Dictionaries------------------------------------------------------------

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiPython)
GAUDI_INSTALL_PYTHON_MODULES()

