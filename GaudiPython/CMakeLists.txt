gaudi_subdir(GaudiPython v14r2)

gaudi_depends_on_subdirs(GaudiAlg GaudiUtils GaudiKernel)

find_package(CLHEP REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(pytools)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiPythonLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiAlgLib PythonLibs CLHEP
                  INCLUDE_DIRS CLHEP PythonLibs
                  PUBLIC_HEADERS GaudiPython)
gaudi_add_module(GaudiPython src/Services/*.cpp
                 LINK_LIBRARIES GaudiPythonLib GaudiUtilsLib)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiPython dict/kernel.h dict/selection_kernel.xml
                     LINK_LIBRARIES GaudiPythonLib GaudiUtilsLib)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
