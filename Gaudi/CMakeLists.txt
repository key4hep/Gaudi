gaudi_subdir(Gaudi v25r3)

gaudi_depends_on_subdirs(GaudiKernel)

#---Executables-------------------------------------------------------------
gaudi_add_executable(Gaudi src/main.cpp LINK_LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()

#---Tests-------------------------------------------------------------------
gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
                 JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/pyjobopts:${CMAKE_CURRENT_SOURCE_DIR}/tests
                 PYTHONPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/python)

