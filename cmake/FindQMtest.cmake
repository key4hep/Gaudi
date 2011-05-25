# - Find QMTest.
#
# This module will define the following variables:
#  QMTEST_EXECUTABLE - the qmtest main script

find_program(QMTEST_EXECUTABLE
  NAMES qmtest
  PATHS ${QMtest_home}/bin
        ${QMtest_home}/Scripts
)

get_filename_component(QMTEST_BINARY_PATH ${QMTEST_EXECUTABLE} PATH)
get_filename_component(QMTEST_PREFIX_PATH ${QMTEST_BINARY_PATH} PATH)

find_path(QMTEST_PYTHON_PATH
  NAMES qm/__init__.py
  PATHS
   ${QMTEST_PREFIX_PATH}/lib/python${Python_config_version_twodigit}/site-packages
   ${QMTEST_PREFIX_PATH}/Lib/site-packages
)

# handle the QUIETLY and REQUIRED arguments and set COOL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QMTest DEFAULT_MSG QMTEST_EXECUTABLE QMTEST_PYTHON_PATH)

mark_as_advanced(QMTEST_EXECUTABLE QMTEST_PYTHON_PATH)


set(QMTEST_ENVIRONMENT QM_home=${QMTEST_PREFIX_PATH})

if(WIN32)
  set(QMTEST_LIBRARY_PATH ${QMTEST_PREFIX_PATH}/Lib/site-packages/pywin32_system32)
endif()
