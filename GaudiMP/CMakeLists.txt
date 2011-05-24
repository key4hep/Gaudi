############################################################################
# CMakeLists.txt file for building GaudiMP
############################################################################
find_package(ROOT)
find_package(Boost)
find_package(PythonLibs)

#---Libraries---------------------------------------------------------------
gaudi_linker_library(GaudiMP Lib/*.cpp
                     LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${ROOT_LIBRARIES}
                     USE_HEADERS Boost ROOT PythonLibs)

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiMP dict/gaudimp_dict.h dict/selection.xml LIBRARIES GaudiMP)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiMP)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
