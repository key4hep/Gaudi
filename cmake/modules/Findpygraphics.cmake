# - Simple module to add the pygraphics directory to the python path
#
# PYGRAPHICS_FOUND
#

set(PYGRAPHICS_FOUND 1)
set(PYGRAPHICS_PYTHON_PATH ${pygraphics_home}/lib/python${Python_config_version_twodigit}/site-packages
    CACHE PATH "Path to the pygraphics LCG package")

mark_as_advanced(PYGRAPHICS_FOUND PYGRAPHICS_PYTHON_PATH)
