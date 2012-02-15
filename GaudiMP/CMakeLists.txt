############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
require(GaudiKernel)

find_package(ROOT)
find_package(PythonLibs)

#---Libraries---------------------------------------------------------------
gaudi_linker_library(GaudiMPLib Lib/*.cpp
                     LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${ROOT_LIBRARIES}
                     USE_HEADERS ROOT PythonLibs)
gaudi_component_library(GaudiMP component/*.cpp
                        LIBRARIES GaudiMPLib ${PYTHON_LIBRARIES} ${ROOT_LIBRARIES})

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiMPLib dict/gaudimp_dict.h dict/selection.xml LIBRARIES GaudiMPLib)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiMP)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
