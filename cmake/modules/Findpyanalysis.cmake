# - Simple module to add the pyanalysis directory to the python path
#
# PYANALYSIS_FOUND
#

set(PYANALYSIS_FOUND 1)
set(PYANALYSIS_PYTHON_PATH ${pyanalysis_home}/lib/python${Python_config_version_twodigit}/site-packages
    CACHE PATH "Path to the pyanalysis LCG package")

mark_as_advanced(PYANALYSIS_FOUND PYANALYSIS_PYTHON_PATH)
