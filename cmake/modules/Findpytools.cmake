# - Simple module to add the pytools directory to the python path
#
# PYTOOLS_FOUND
#

set(PYTOOLS_FOUND 1)
set(PYTOOLS_PYTHON_PATH ${pytools_home}/lib/python${Python_config_version_twodigit}/site-packages
    CACHE PATH "Path to the pytools LCG package")

mark_as_advanced(PYTOOLS_FOUND PYTOOLS_PYTHON_PATH)
