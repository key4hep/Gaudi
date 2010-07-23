INCLUDE(Configuration)

SET(QMtest_native_version ${QMtest_config_version}_python${Python_config_version_twodigit} )
SET(QMtest_home ${LCG_external}/QMtest/${QMtest_native_version}/${LCG_system})

SET(QMtest_FOUND 1)

SET(QMtest_environment QM_home=${QMtest_home}
                       PATH+=${QMtest_home}/bin
                       LD_LIBRARY_PATH+=${QMtest_home}/lib
                       PYTHONPATH+=${QMtest_home}/lib/python${Python_config_version_twodigit}/site-packages 
   )

 
