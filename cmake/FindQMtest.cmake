# - Find QMTest.
#
# This module will define the following variables:
#  QMTest_EXECUTABLE - the qmtest main script

find_program(QMTest_EXECUTABLE
  NAMES qmtest
  PATHS ${QMtest_home}/bin
        ${QMTest_home}/Scripts
)

get_filename_component(QMTest_BINARY_PATH ${QMTest_EXECUTABLE} PATH)
get_filename_component(QMTest_PREFIX_PATH ${QMTest_BINARY_PATH} PATH)

find_path(QMTest_PYTHON_PATH
  NAMES qm/__init__.py
  PATHS
   ${QMTest_PREFIX_PATH}/lib/python${Python_config_version_twodigit}/site-packages
   ${QMTest_PREFIX_PATH}/Lib/site-packages
)

# handle the QUIETLY and REQUIRED arguments and set COOL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QMTest DEFAULT_MSG QMTest_EXECUTABLE QMTest_PYTHON_PATH)

mark_as_advanced(QMTest_EXECUTABLE QMTest_PYTHON_PATH)


set(QMtest_environment QM_home=${QMTest_PREFIX_PATH}
                       PATH+=${QMTest_BINARY_PATH}
                       PYTHONPATH+=${QMTest_PYTHON_PATH}
   )

if(WIN32)
  set(QMtest_environment ${QMtest_environment}
                         PATH+=${QMTest_PREFIX_PATH}/Lib/site-packages/pywin32_system32
     )
endif()
