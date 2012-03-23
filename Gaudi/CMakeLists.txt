############################################################################
# CMakeLists.txt file for building Gaudi
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

#---Executables-------------------------------------------------------------
gaudi_add_executable(Gaudi main.cpp LINK_LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
GAUDI_INSTALL_SCRIPTS()
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/python)")
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/scripts)")

#---Tests-------------------------------------------------------------------
gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
                 JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/pyjobopts:${CMAKE_CURRENT_SOURCE_DIR}/tests
                 PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/python)

