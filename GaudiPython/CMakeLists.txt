#############################################
# CMakeList.txt file for building GaudiPython
# @author Pere Mato, CERN
#############################################

GAUDI_USE_PACKAGE(ROOT)
GAUDI_USE_PACKAGE(AIDA)
GAUDI_USE_PACKAGE(Python)
GAUDI_USE_PACKAGE(CLHEP)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS} 
                     ${AIDA_INCLUDE_DIRS} ${Python_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}) 
LINK_DIRECTORIES( ${GAUDI_LIBRARY_DIRS} ${Python_LIBRARY_DIRS}) 


#---Linker Library-------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiPythonLib Lib/*.cpp GaudiKernel GaudiAlgLib ${Python_LIBRARIES})

#---Component Library----------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiPython Services/*.cpp GaudiPythonLib)

#---Dictionary-----------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiPython dict/kernel.h dict/selection_kernel.xml GaudiPythonLib)

#---Installation---------------------------------------------------------------
GAUDI_INSTALL_HEADERS()
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()


