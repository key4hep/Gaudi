###############################################
# CMakeLists.txt file for building Gaudi
# @author Pere Mato, CERN
###############################################


GAUDI_USE_PACKAGE(GaudiKernel)

#---Gaudi main program ----------------------------------------------------------------------
GAUDI_EXECUTABLE( Gaudi.exe main.cpp GaudiKernel)

#---Installation-----------------------------------------------------------------------------
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()

INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/python)")
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/scripts)")
