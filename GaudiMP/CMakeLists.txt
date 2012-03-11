############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
require(GaudiKernel)

find_package(ROOT)
find_package(PythonLibs)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiMPLib Lib/*.cpp
                     LINK_LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${ROOT_LIBRARIES}
                     INCLUDE_DIRS ROOT PythonLibs)
gaudi_add_module(GaudiMP component/*.cpp
                        LINK_LIBRARIES GaudiMPLib ${PYTHON_LIBRARIES} ${ROOT_LIBRARIES})

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiMPLib dict/gaudimp_dict.h dict/selection.xml LIBRARIES GaudiMPLib)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiMP)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
