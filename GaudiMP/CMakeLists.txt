############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
require(GaudiKernel)

find_package(ROOT)
find_package(PythonLibs)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiMPLib Lib/*.cpp
                     LINK_LIBRARIES GaudiKernel PythonLibs ROOT
                     INCLUDE_DIRS ROOT PythonLibs)
gaudi_add_module(GaudiMP component/*.cpp
                        LINK_LIBRARIES GaudiMPLib PythonLibs ROOT)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiMPLib dict/gaudimp_dict.h dict/selection.xml LINK_LIBRARIES GaudiMPLib)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiMP)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
