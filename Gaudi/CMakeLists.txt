gaudi_subdir(Gaudi v26r1)

gaudi_depends_on_subdirs(GaudiKernel)

#---Executables-------------------------------------------------------------
gaudi_add_executable(Gaudi src/main.cpp LINK_LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()

#---Tests-------------------------------------------------------------------
# FIXME: these variables must be unset in the tests for compatibility with CMT
gaudi_build_env(UNSET GAUDIAPPNAME
                UNSET GAUDIAPPVERSION)

gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
                 JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/pyjobopts:${CMAKE_CURRENT_SOURCE_DIR}/tests
                 PYTHONPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/python)

gaudi_add_test(nose
               COMMAND nosetests --with-doctest -v
                  ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose
                  ${CMAKE_CURRENT_SOURCE_DIR}/python/Gaudi)
