# Simplified wrapper for the standard Python cmake configuration files

find_package(PythonInterp)
find_package(PythonLibs)

set(PYTHON_FOUND 1)

set(Python_INCLUDE_DIRS ${PYTHON_INCLUDE_DIR})
