# - Find python interpreter (LCG)
# This module finds if Python interpreter is installed and determines where the
# executables are. This code sets the following variables:
#
#  PYTHONINTERP_FOUND - Was the Python executable found
#  PYTHON_EXECUTABLE  - path to the Python interpreter
#

include(PythonLCGCommon)

set(PYTHONINTERP_FOUND 1)

FIND_PROGRAM(PYTHON_EXECUTABLE
  NAMES python
  PATHS
    ${Python_home}/bin
    ${Python_home}
)

mark_as_advanced(PYTHON_EXECUTABLE)
