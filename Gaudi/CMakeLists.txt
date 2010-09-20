############################################################################
# CMakeLists.txt file for building Gaudi
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(Gaudi main.cpp LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()

INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/python)")
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/scripts)")
