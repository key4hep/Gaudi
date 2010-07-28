#############################################
# CMakeList.txt file for building GaudiPython
# @author Pere Mato, CERN
#############################################

GAUDI_USE_PACKAGE(Python)
GAUDI_USE_PACKAGE(CLHEP)
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiAlg)

#---Dictionary-----------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiPython dict/kernel.h dict/selection_kernel.xml GaudiPythonLib)

#---Linker Library-------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiPythonLib Lib/*.cpp GaudiKernel GaudiAlgLib ${Python_LIBRARIES})

#---Component Library----------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiPython Services/*.cpp GaudiPythonLib)


#---Installation---------------------------------------------------------------
GAUDI_INSTALL_HEADERS()
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()


