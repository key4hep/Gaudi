gaudi_subdir(GaudiPython v12r7)

gaudi_depends_on_subdirs(GaudiAlg GaudiUtils GaudiKernel)

find_package(CLHEP)
find_package(PythonLibs)
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

if(GAUDI_BUILD_TESTS)
  gaudi_add_library(GPyTest src/Test/*.cpp
                    LINK_LIBRARIES GaudiKernel PythonLibs CLHEP
                    NO_PUBLIC_HEADERS)
  gaudi_add_dictionary(GPyTest src/Test/test.h src/Test/test_selection.xml
                       LINK_LIBRARIES GaudiKernel PythonLibs CLHEP)
  include_directories(src/Test)
endif()

#---Installation------------------------------------------------------------
gaudi_install_python_modules()

