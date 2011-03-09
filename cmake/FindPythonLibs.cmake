# - Find python libraries (LCG)
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHONLIBS_FOUND     = have the Python libs been found
#  PYTHON_LIBRARIES     = path to the python library
#  PYTHON_INCLUDE_DIR   = path to where Python.h is found
#  PYTHON_DEBUG_LIBRARIES = path to the debug library
#

include(PythonLCGCommon)

set(PYTHONLIBS_FOUND 1)

find_path(PYTHON_INCLUDE_DIR
  NAMES
    Python.h
  PATHS
    ${Python_home}/include/python${Python_config_version_twodigit}
    ${Python_home}/include
)

string(REPLACE "." "" Python_config_version_twodigit_no_dot ${Python_config_version_twodigit})
find_library(PYTHON_LIBRARY
  NAMES python${Python_config_version_twodigit_no_dot} python${Python_config_version_twodigit}
  PATHS
    ${Python_home}/libs
    ${Python_home}/lib
  # Avoid finding the .dll in the PATH.  We want the .lib.
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_DEFAULT_PATH
)

set(PYTHON_LIBRARIES ${PYTHON_LIBRARY})
set(PYTHON_DEBUG_LIBRARIES ${PYTHON_LIBRARY})
