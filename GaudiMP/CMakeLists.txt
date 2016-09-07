gaudi_subdir(GaudiMP v3r5)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg)

find_package(ROOT COMPONENTS Net RIO Thread)
find_package(PythonLibs)

# Hide some Boost compile time warnings
find_package(Boost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiMPLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiKernel PythonLibs ROOT
                  INCLUDE_DIRS ROOT PythonLibs
                  PUBLIC_HEADERS GaudiMP)
gaudi_add_module(GaudiMP src/component/*.cpp
                 LINK_LIBRARIES GaudiMPLib PythonLibs ROOT GaudiAlgLib)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiMP dict/gaudimp_dict.h dict/selection.xml LINK_LIBRARIES GaudiMPLib)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()
