# - Find python interpreter
# This module finds if Python interpreter is installed and determines where the
# executables are. This code sets the following variables:
#
#  PYTHONINTERP_FOUND - Was the Python executable found
#  PYTHON_EXECUTABLE  - path to the Python interpreter
#

#=============================================================================
# Copyright 2005-2010 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# The following was added for Debian by Kai Wasserbäch
# <debian@carbon-project.org> in 2010 and based on suggestions and patches by
# Didier Raboud <didier@raboud.com> and Jakub Wilk <jwilk@debian.org> to fix the
# following bugs: #569321 and #580503.
# For further information please see the Debian.NEWS file for cmake-data.
IF(CMAKE_USE_PYTHON_VERSION)
  SET(CMAKE_PYTHON_VERSIONS ${CMAKE_USE_PYTHON_VERSION})
ELSE(CMAKE_USE_PYTHON_VERSION)
  SET(CMAKE_PYTHON_VERSIONS 2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0 1.6 1.5)

  FIND_PROGRAM(PYVERSIONS_EXE
    NAMES pyversions
    )
  IF(PYVERSIONS_EXE)
    EXECUTE_PROCESS(COMMAND pyversions -dv
      OUTPUT_VARIABLE _CMAKE_PYTHON_VERSIONS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    LIST(INSERT CMAKE_PYTHON_VERSIONS 0 ${_CMAKE_PYTHON_VERSIONS})
    LIST(REMOVE_DUPLICATES CMAKE_PYTHON_VERSIONS)
  ENDIF(PYVERSIONS_EXE)
ENDIF(CMAKE_USE_PYTHON_VERSION)

FOREACH(_CURRENT_VERSION ${CMAKE_PYTHON_VERSIONS})
  FIND_PROGRAM(PYTHON_EXECUTABLE
    NAMES python${_CURRENT_VERSION}
    PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]
    )
ENDFOREACH(_CURRENT_VERSION)

# handle the QUIETLY and REQUIRED arguments and set PYTHONINTERP_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonInterp DEFAULT_MSG PYTHON_EXECUTABLE)

MARK_AS_ADVANCED(PYTHON_EXECUTABLE)

