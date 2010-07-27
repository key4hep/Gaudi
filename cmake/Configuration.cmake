#------------Based on LCGCMT_59-------------------------
SET(LCG_home /afs/cern.ch/sw/lcg)

IF (CMAKE_SYSTEM_NAME MATCHES Linux) 
  SET(LCG_platform x86_64-slc5-gcc43-dbg)
  SET(LCG_basesystem x86_64-slc5-gcc43)
ELSEIF( CMAKE_SYSTEM_NAME MATCHES Darwin)
  SET(LCG_platform x86_64-mac106-gcc42-dbg)
  SET(LCG_basesystem x86_64-mac106-gcc42)
ENDIF()

SET(LCG_system ${LCG_basesystem}-opt)


SET(LCG_releases ${LCG_home}/app/releases)
SET(LCG_external ${LCG_home}/external)

set(CMAKE_CXX_COMPILER g++)

SET(COOL_config_version            COOL_2_8_6d)
SET(CORAL_config_version           CORAL_2_3_10a)
SET(POOL_config_version            POOL_2_9_9a)
SET(RELAX_config_version           RELAX_1_2_1f)
SET(ROOT_config_version            5.26.00c)

SET(AIDA_config_version            3.2.1)
SET(CLHEP_config_version           1.9.4.7)
SET(GSL_config_version             1.10)
SET(Boost_config_version           1.42.0)
SET(Boost_file_version             1_42)
SET(HepPDT_config_version          2.06.01)

SET(py_config_version              1.2.1)
SET(pydot_config_version           1.0.2)
SET(pyanalysis_config_version      1.2)
SET(pygraphics_config_version      1.1) 
SET(pyminuit_config_version        0.0.1)
SET(pytools_config_version         1.1)

SET(Python_config_version          2.6.5)
SET(Python_config_version_twodigit 2.6)

SET(GCCXML_config_version          0.9.0_20100114)
SET(uuid_config_version            1.38p1)
SET(XercesC_config_version         3.1.1)
SET(XercesC_file_version           3_1_1)
SET(QMtest_config_version          2.4p1)
