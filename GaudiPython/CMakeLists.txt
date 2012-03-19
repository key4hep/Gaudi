############################################################################
# CMakeLists.txt file for building GaudiPython
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiAlg GaudiUtils GaudiKernel)

find_package(CLHEP)
find_package(PythonLibs)
find_package(pytools)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiPythonLib Lib/*.cpp
                     LINK_LIBRARIES GaudiAlgLib ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES}
                     INCLUDE_DIRS CLHEP PythonLibs)
gaudi_add_module(GaudiPython Services/*.cpp LINK_LIBRARIES GaudiPythonLib GaudiUtilsLib)

gaudi_add_dictionary(GaudiPython dict/kernel.h  dict/selection_kernel.xml LINK_LIBRARIES GaudiPythonLib GaudiUtilsLib)

if(BUILD_TESTS)
  gaudi_add_library(GPyTest Test/*.cpp
                       LINK_LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES})
  gaudi_add_dictionary(GPyTest src/Test/test.h src/Test/test_selection.xml LINK_LIBRARIES GaudiKernel ${PYTHON_LIBRARIES} ${CLHEP_LIBRARIES})
  include_directories(src/Test)
endif()

#---Dictionaries------------------------------------------------------------

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiPython)
GAUDI_INSTALL_PYTHON_MODULES()

