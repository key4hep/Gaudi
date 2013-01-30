gaudi_subdir(GaudiMP v1r7)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(ROOT)
find_package(PythonLibs)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiMPLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiKernel PythonLibs ROOT
                  INCLUDE_DIRS ROOT PythonLibs
                  PUBLIC_HEADERS GaudiMP)
gaudi_add_module(GaudiMP src/component/*.cpp
                 LINK_LIBRARIES GaudiMPLib PythonLibs ROOT)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiMPLib dict/gaudimp_dict.h dict/selection.xml LINK_LIBRARIES GaudiMPLib)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()
