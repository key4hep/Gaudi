############################################################################
# CMakeLists.txt file for building Gaudi
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(Gaudi main.cpp LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/python)")
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${python_cmd} -m compileall  ${CMAKE_INSTALL_PREFIX}/scripts)")

#---Tests-------------------------------------------------------------------
set( QMtests
     bug_34769	bug_37479	bug_39846	cmdline_conf import_opts
     bug_35201	bug_37488	bug_42428	conf_to_str		import_py_err
     bug_35347	bug_38194	bug_45517	conf_user_autoapply	import_py	propagate_properties
     bug_38206	bug_61144	conf_user_passive_dep	override1	purge
     bug_39809	case_sensitivity export_oldopts		override2	reference)

foreach( test ${QMtests})
  GAUDI_QMTEST_TEST(gaudi.${test} ENVIRONMENT
                    JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/pyjobopts
                    PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/python )
endforeach()
#---Combined Tests-----------------------------------------------------------
GAUDI_QMTEST_TEST( gaudi.pickle TESTS  gaudi.pickle_write gaudi.pickle_read  ENVIRONMENT
                      JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/pyjobopts
                      PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/python )
GAUDI_QMTEST_TEST( gaudi.bug_36742 TESTS  gaudi.bug_36742 gaudi.bug_36742b ENVIRONMENT
                      JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/pyjobopts
                      PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/python )

