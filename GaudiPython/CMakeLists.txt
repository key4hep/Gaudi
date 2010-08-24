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

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiPythonLib Lib/*.cpp GaudiAlgLib ${Python_LIBRARIES} ${CLHEP_LIBRARIES})
GAUDI_LINKER_LIBRARY(GPyTest Test/*.cpp GaudiPythonLib GaudiUtilsLib)
GAUDI_COMPONENT_LIBRARY(GaudiPython Services/*.cpp GaudiPythonLib GaudiUtilsLib)

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiPython dict/kernel.h  dict/selection_kernel.xml  GaudiPythonLib GaudiUtilsLib)
REFLEX_BUILD_DICTIONARY(test_GPyTest src/Test/test.h  src/Test/test_selection.xml  GaudiPythonLib GaudiUtilsLib)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiPython)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()

