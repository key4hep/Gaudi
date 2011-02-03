############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiPython)
GAUDI_USE_PACKAGE(Python)
GAUDI_USE_PACKAGE(ROOT)

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiMP Lib/*.cpp LIBRARIES GaudiKernel ${Python_LIBRARIES} ${ROOT_LIBRARIES})

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiMP dict/gaudimp_dict.h dict/selection.xml LIBRARIES GaudiMP)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiMP)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
